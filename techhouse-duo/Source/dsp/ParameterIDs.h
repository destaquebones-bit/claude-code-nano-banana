#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Every automatable parameter, in one place.
//
// Both modes share one parameter set: the plugin is a single binary (see
// LinkBus.h for why), so a Kick-mode instance simply ignores the Bass-mode
// parameters and vice versa. The editor shows only the ones that apply.
namespace ParamIDs
{
    // --- Global ---
    static constexpr auto mode        = "mode";          // 0 = Bass, 1 = Kick
    static constexpr auto linkChannel = "linkChannel";   // A..H
    static constexpr auto linkEnable  = "linkEnable";
    static constexpr auto inputHpf    = "inputHpf";
    static constexpr auto outputGain  = "outputGain";
    static constexpr auto dryWet      = "dryWet";
    static constexpr auto listenMode  = "listenMode";    // hear only what is being removed
    static constexpr auto bypass      = "bypass";

    // --- Bass mode ---
    static constexpr auto tameDepth     = "tameDepth";
    static constexpr auto tameTolerance = "tameTolerance";
    static constexpr auto tameMaxCut    = "tameMaxCut";
    static constexpr auto tameRelease   = "tameRelease";
    static constexpr auto harmonicCount = "harmonicCount";
    static constexpr auto mudDepth      = "mudDepth";
    static constexpr auto noteCompAmount= "noteCompAmount";
    static constexpr auto duckAmount    = "duckAmount";
    static constexpr auto duckThreshold = "duckThreshold";
    static constexpr auto duckRelease   = "duckRelease";
    static constexpr auto exciteAmount  = "exciteAmount";
    static constexpr auto exciteBalance = "exciteBalance";
    static constexpr auto monoBelow     = "monoBelow";

    // --- Kick mode ---
    static constexpr auto kickSubFreq  = "kickSubFreq";
    static constexpr auto kickTopFreq  = "kickTopFreq";
    static constexpr auto kickSubGain  = "kickSubGain";
    static constexpr auto kickTopGain  = "kickTopGain";
    static constexpr auto kickTailAmt  = "kickTailAmt";
    static constexpr auto kickTailMs   = "kickTailMs";
    static constexpr auto kickAttack   = "kickAttack";
    static constexpr auto kickBoxiness = "kickBoxiness";
    static constexpr auto kickBassAware= "kickBassAware"; // reverse link: step aside for the bass note
    static constexpr auto kickDrive    = "kickDrive";     // transformer-style saturation, body band only

    inline juce::StringArray linkChannelNames()
    {
        return { "A", "B", "C", "D", "E", "F", "G", "H" };
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using Range = juce::NormalisableRange<float>;
        using Attr = juce::AudioParameterFloatAttributes;
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

        auto pct = [] { return Range (0.0f, 100.0f, 0.1f); };
        auto freq = [] (float lo, float hi)
        {
            Range r (lo, hi, 0.01f);
            r.setSkewForCentre (juce::jlimit (lo + 1.0f, hi - 1.0f, std::sqrt (lo * hi)));
            return r;
        };

        auto addFloat = [&p] (const char* id, const juce::String& name, Range r, float def, const juce::String& label)
        {
            p.push_back (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID (id, 1), name, r, def, Attr().withLabel (label)));
        };
        auto addBool = [&p] (const char* id, const juce::String& name, bool def)
        {
            p.push_back (std::make_unique<juce::AudioParameterBool> (juce::ParameterID (id, 1), name, def));
        };

        // Global
        p.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID (mode, 1), "Mode", juce::StringArray { "Bass", "Kick" }, 0));
        p.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID (linkChannel, 1), "Link Channel", linkChannelNames(), 0));
        addBool (linkEnable, "Link Enable", true);
        addFloat (inputHpf, "Input HPF", freq (10.0f, 60.0f), 22.0f, "Hz");
        addFloat (outputGain, "Output", Range (-24.0f, 24.0f, 0.01f), 0.0f, "dB");
        addFloat (dryWet, "Dry/Wet", pct(), 100.0f, "%");
        addBool (listenMode, "Listen (removed only)", false);
        addBool (bypass, "Bypass", false);

        // Bass
        addFloat (tameDepth, "Tame Depth", pct(), 55.0f, "%");
        addFloat (tameTolerance, "Tame Tolerance", Range (0.0f, 12.0f, 0.01f), 3.0f, "dB");
        addFloat (tameMaxCut, "Tame Max Cut", Range (0.0f, 24.0f, 0.01f), 10.0f, "dB");
        addFloat (tameRelease, "Tame Release", Range (20.0f, 400.0f, 0.1f, 0.5f), 90.0f, "ms");
        p.push_back (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID (harmonicCount, 1), "Harmonics Tracked", 3, 10, 8));
        addFloat (mudDepth, "Mud Control", pct(), 35.0f, "%");
        addFloat (noteCompAmount, "Note Levelling", pct(), 0.0f, "%");
        addFloat (duckAmount, "Kick Ducking", pct(), 50.0f, "%");
        addFloat (duckThreshold, "Duck Threshold", Range (-60.0f, -10.0f, 0.1f), -40.0f, "dB");
        addFloat (duckRelease, "Duck Release", Range (20.0f, 400.0f, 0.1f, 0.5f), 90.0f, "ms");
        addFloat (exciteAmount, "Harmonic Excite", pct(), 0.0f, "%");
        addFloat (exciteBalance, "Excite Balance", pct(), 50.0f, "%");
        addFloat (monoBelow, "Mono Below", freq (20.0f, 300.0f), 120.0f, "Hz");

        // Kick
        addFloat (kickSubFreq, "Sub/Body Split", freq (60.0f, 300.0f), 120.0f, "Hz");
        addFloat (kickTopFreq, "Body/Click Split", freq (600.0f, 6000.0f), 1500.0f, "Hz");
        addFloat (kickSubGain, "Sub Gain", Range (-18.0f, 12.0f, 0.01f), 0.0f, "dB");
        addFloat (kickTopGain, "Click Gain", Range (-18.0f, 12.0f, 0.01f), 0.0f, "dB");
        addFloat (kickTailAmt, "Tail Tighten", pct(), 0.0f, "%");
        addFloat (kickTailMs, "Tail Length", Range (20.0f, 600.0f, 0.1f, 0.5f), 130.0f, "ms");
        addFloat (kickAttack, "Attack Shape", Range (-100.0f, 100.0f, 0.1f), 0.0f, "%");
        addFloat (kickBoxiness, "Boxiness Tame", pct(), 30.0f, "%");
        addFloat (kickBassAware, "Bass-Aware Notch", pct(), 0.0f, "%");
        // Off by default: this plugin's job is to take mud out, so anything
        // that adds harmonic content has to be opted into deliberately.
        addFloat (kickDrive, "Drive", pct(), 0.0f, "%");

        return { p.begin(), p.end() };
    }
}
