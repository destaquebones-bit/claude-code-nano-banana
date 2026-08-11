#pragma once
#include "DspCommon.h"

// Fixes the "one-note bass" problem: in almost every rolling bassline one or
// two notes come out noticeably louder or muddier than the rest -- because of a
// resonance in the synth patch, an uneven sample, or just where the notes land
// against the room/monitoring. Producers currently fix this by hand with clip
// gain, note by note.
//
// This builds a running level profile per MIDI note as the track plays, then
// applies the gain that pulls each note back toward the average of all the
// notes actually used. Because it keys off the tracked pitch rather than
// frequency bands, it levels the *note*, which no static or spectral tool can
// do.
//
// It's deliberately slow and conservative: it only learns from stable, confident
// notes, needs several visits to a note before trusting its level, and clamps
// how far it will push. A wrong guess here would be very audible.
class NoteLevelCompensator
{
public:
    static constexpr int numNotes = 128;
    static constexpr int minObservations = 3;

    void prepare (double sampleRate)
    {
        levelSmoother.prepare (sampleRate);
        levelSmoother.setTimes (20.0f, 20.0f);
        gainSmoother.prepare (sampleRate);
        gainSmoother.setTimes (60.0f, 60.0f);
        samplesPerNoteUpdate = juce::jmax (1, (int) (sampleRate * 0.02)); // 20ms granularity
        resetLearning();
    }

    void resetLearning()
    {
        for (int i = 0; i < numNotes; ++i)
        {
            noteLevelDb[i] = 0.0f;
            observations[i] = 0;
        }
        currentNote = -1;
        stableSamples = 0;
        counter = 0;
        compensationDb = 0.0f;
    }

    struct Params
    {
        float amount = 0.0f;     // 0-1
        float maxCorrectionDb = 6.0f;
        float adaptSpeed = 0.08f; // per-observation blend toward the newest measurement
    };

    void setParams (const Params& p) { params = p; }

    // Feed one sample of the (already cleaned) bass plus the current pitch
    // estimate. Returns the linear gain to apply.
    float processSample (float monoSample, float fundamentalHz, float confidence)
    {
        const float envDb = DspCommon::gainToDb (levelSmoother.process (std::abs (monoSample)));

        if (++counter >= samplesPerNoteUpdate)
        {
            counter = 0;
            updateLearning (fundamentalHz, confidence, envDb);
        }

        return DspCommon::dbToGain (gainSmoother.process (compensationDb));
    }

    float getCompensationDb() const { return compensationDb; }

    // How uneven the bassline is overall, in dB between the quietest and
    // loudest learned note. This is the number that tells a producer whether
    // they have a one-note-bass problem at all.
    float getNoteSpreadDb() const
    {
        float lo = 1.0e9f, hi = -1.0e9f;
        int n = 0;
        for (int i = 0; i < numNotes; ++i)
        {
            if (observations[i] < minObservations)
                continue;
            lo = juce::jmin (lo, noteLevelDb[i]);
            hi = juce::jmax (hi, noteLevelDb[i]);
            ++n;
        }
        return n >= 2 ? hi - lo : 0.0f;
    }

    int getLearnedNoteCount() const
    {
        int n = 0;
        for (int i = 0; i < numNotes; ++i)
            if (observations[i] >= minObservations)
                ++n;
        return n;
    }

    int getCurrentNote() const { return currentNote; }

private:
    void updateLearning (float fundamentalHz, float confidence, float envDb)
    {
        const int note = confidence > 0.6f ? DspCommon::freqToMidiNote (fundamentalHz) : -1;

        if (note < 0 || note >= numNotes || envDb < -50.0f)
        {
            currentNote = note;
            stableSamples = 0;
            compensationDb = 0.0f;
            return;
        }

        if (note != currentNote)
        {
            currentNote = note;
            stableSamples = 0;
            return;
        }

        // Only learn once the note has held for a moment -- the attack and any
        // pitch glide at the start would otherwise pollute the measurement.
        if (++stableSamples < 3)
            return;

        if (observations[note] == 0)
            noteLevelDb[note] = envDb;
        else
            noteLevelDb[note] += params.adaptSpeed * (envDb - noteLevelDb[note]);

        if (observations[note] < 1000)
            ++observations[note];

        compensationDb = computeCorrection (note);
    }

    float computeCorrection (int note) const
    {
        if (params.amount <= 0.0f || observations[note] < minObservations)
            return 0.0f;

        double sum = 0.0;
        int n = 0;
        for (int i = 0; i < numNotes; ++i)
        {
            if (observations[i] < minObservations)
                continue;
            sum += noteLevelDb[i];
            ++n;
        }
        if (n < 2)
            return 0.0f;

        const float average = (float) (sum / n);
        const float correction = (average - noteLevelDb[note]) * params.amount;
        return juce::jlimit (-params.maxCorrectionDb, params.maxCorrectionDb, correction);
    }

    Params params;
    DspCommon::OnePoleEnvelope levelSmoother, gainSmoother;
    float noteLevelDb[numNotes] {};
    int observations[numNotes] {};
    int currentNote = -1, stableSamples = 0, counter = 0, samplesPerNoteUpdate = 882;
    float compensationDb = 0.0f;
};
