#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <array>

// Cross-instance communication bus.
//
// WHY THIS EXISTS: the whole point of this plugin is that the Kick instance and
// the Bass instance cooperate. Instead of routing the kick's raw audio into the
// bass track as a sidechain (fiddly in Live, and the bass side then has to
// re-analyse audio the kick instance already analysed), each instance publishes
// its *analysis* here and reads the other's.
//
// WHY IT'S ONE PLUGIN WITH A MODE SWITCH, NOT TWO PLUGINS: two separate .vst3
// binaries are two separately-loaded modules, each with its own copy of any
// static/singleton state -- they could not see each other's frames. Shipping a
// single binary with a Kick/Bass mode parameter means both instances live in
// the same module and genuinely share this bus. That constraint drove the
// product shape, not the other way around.
//
// TIMING: frames are stamped with the host timeline sample position they
// describe, and readers ask for a specific position rather than "the latest".
// That makes the link immune to track processing order -- Live may process the
// bass track before or after the kick track in any given buffer, and the bass
// side still gets the kick data belonging to the exact samples it is working
// on. The bass side runs a lookahead delay anyway (for pitch tracking), so the
// positions it asks about are comfortably in the past by the time it asks.
namespace Link
{
    static constexpr int numChannels = 8;   // "Link A".."Link H" -- lets several kick/bass pairs coexist
    static constexpr int numBands = 8;      // spectral resolution shared between the two sides
    static constexpr int ringSize = 512;    // frames; at one frame per 64 samples this is ~0.7s of history

    // Band centre frequencies both sides agree on. Fixed (not note-relative) so
    // the two instances are always talking about the same thing.
    static constexpr std::array<float, numBands> bandCentres
        { 45.0f, 65.0f, 95.0f, 140.0f, 200.0f, 290.0f, 420.0f, 600.0f };

    struct KickFrame
    {
        float bandDb[numBands] {};    // kick energy per band, dB
        float transient = 0.0f;       // 0-1, how much of a hit is happening right now
        float broadbandDb = -120.0f;
    };

    struct BassFrame
    {
        float fundamentalHz = 0.0f;   // 0 when the tracker isn't confident
        float confidence = 0.0f;      // 0-1
        float broadbandDb = -120.0f;
    };

    // Seqlock-protected slot: the audio threads of two different tracks write
    // and read these concurrently, and we must not block either one. Writers
    // bump `seq` to odd, write, bump to even; readers retry if `seq` changed
    // across the read or was odd. Frames are small PODs so retries are cheap
    // and vanishingly rare in practice.
    template <typename FrameType>
    struct Slot
    {
        std::atomic<uint32_t> seq { 0 };
        std::atomic<int64_t> position { INT64_MIN };
        FrameType frame;

        void write (int64_t pos, const FrameType& f)
        {
            seq.fetch_add (1, std::memory_order_acquire);
            position.store (pos, std::memory_order_relaxed);
            frame = f;
            seq.fetch_add (1, std::memory_order_release);
        }

        bool read (int64_t& posOut, FrameType& out) const
        {
            const uint32_t before = seq.load (std::memory_order_acquire);
            if (before & 1u)
                return false;
            posOut = position.load (std::memory_order_relaxed);
            out = frame;
            return seq.load (std::memory_order_acquire) == before;
        }
    };

    template <typename FrameType>
    struct Ring
    {
        std::array<Slot<FrameType>, ringSize> slots;
        std::atomic<uint64_t> writeIndex { 0 };
        std::atomic<int> publisherCount { 0 };

        void publish (int64_t position, const FrameType& f)
        {
            const uint64_t i = writeIndex.fetch_add (1, std::memory_order_relaxed);
            slots[(size_t) (i % ringSize)].write (position, f);
        }

        // Finds the most recent frame at or before `position`. Returns false if
        // nothing suitable is in the ring (link partner absent, or the position
        // is older than our history).
        bool readAt (int64_t position, FrameType& out) const
        {
            int64_t bestPos = INT64_MIN;
            FrameType best {};
            bool found = false;

            for (size_t i = 0; i < ringSize; ++i)
            {
                int64_t slotPos = INT64_MIN;
                FrameType candidate {};
                if (! slots[i].read (slotPos, candidate))
                    continue;
                if (slotPos == INT64_MIN || slotPos > position)
                    continue;
                if (slotPos > bestPos)
                {
                    bestPos = slotPos;
                    best = candidate;
                    found = true;
                }
            }

            if (found)
                out = best;
            return found;
        }

        // Fallback for when the transport is stopped and positions are
        // meaningless: just take whatever was written most recently.
        bool readLatest (FrameType& out) const
        {
            const uint64_t i = writeIndex.load (std::memory_order_relaxed);
            if (i == 0)
                return false;
            int64_t pos = 0;
            return slots[(size_t) ((i - 1) % ringSize)].read (pos, out);
        }
    };

    struct Channel
    {
        Ring<KickFrame> kick;
        Ring<BassFrame> bass;
    };

    // Single shared instance. Safe as a function-local static (thread-safe
    // initialisation is guaranteed since C++11), and correct here precisely
    // because every instance of this plugin lives in the same loaded binary.
    inline Channel& getChannel (int index)
    {
        static std::array<Channel, numChannels> channels;
        return channels[(size_t) juce::jlimit (0, numChannels - 1, index)];
    }

    // Instances register/unregister so each side can tell the user whether its
    // partner is actually present -- "Link A: waiting for a Kick instance" is
    // far better than silently doing nothing.
    struct Registration
    {
        Registration (int channelIndex, bool isKickMode)
            : channel (channelIndex), kickMode (isKickMode)
        {
            bump (1);
        }

        ~Registration() { bump (-1); }

        void moveTo (int newChannel, bool newKickMode)
        {
            if (newChannel == channel && newKickMode == kickMode)
                return;
            bump (-1);
            channel = newChannel;
            kickMode = newKickMode;
            bump (1);
        }

        int channel;
        bool kickMode;

    private:
        void bump (int delta)
        {
            auto& c = getChannel (channel);
            (kickMode ? c.kick.publisherCount : c.bass.publisherCount)
                .fetch_add (delta, std::memory_order_relaxed);
        }
    };

    inline bool isKickPresent (int channel)
    {
        return getChannel (channel).kick.publisherCount.load (std::memory_order_relaxed) > 0;
    }

    inline bool isBassPresent (int channel)
    {
        return getChannel (channel).bass.publisherCount.load (std::memory_order_relaxed) > 0;
    }
}
