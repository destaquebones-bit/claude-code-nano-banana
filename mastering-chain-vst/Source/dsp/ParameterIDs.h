#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Central list of every automatable parameter in the plugin, plus the
// APVTS layout builder. Keeping IDs here (instead of scattered string
// literals) means renaming/adding a parameter is a one-file change.
namespace ParamIDs
{
    // --- Input ---
    static constexpr auto inputGainDb   = "inputGainDb";
    static constexpr auto inputInvert   = "inputInvert";

    // --- Analog EQ (Pultec-style low end, 2 mid bells, air shelf) ---
    static constexpr auto eqBypass        = "eqBypass";
    static constexpr auto eqLowFreq       = "eqLowFreq";
    static constexpr auto eqLowBoostDb    = "eqLowBoostDb";
    static constexpr auto eqLowCutDb      = "eqLowCutDb";
    static constexpr auto eqLowMidFreq    = "eqLowMidFreq";
    static constexpr auto eqLowMidGainDb  = "eqLowMidGainDb";
    static constexpr auto eqLowMidQ       = "eqLowMidQ";
    static constexpr auto eqHighMidFreq   = "eqHighMidFreq";
    static constexpr auto eqHighMidGainDb = "eqHighMidGainDb";
    static constexpr auto eqHighMidQ      = "eqHighMidQ";
    static constexpr auto eqAirFreq       = "eqAirFreq";
    static constexpr auto eqAirGainDb     = "eqAirGainDb";
    static constexpr auto eqIronAmount    = "eqIronAmount";   // transformer-style coloration

    // --- Glue (bus) compressor ---
    static constexpr auto compBypass    = "compBypass";
    static constexpr auto compThreshold = "compThreshold";
    static constexpr auto compRatio     = "compRatio";
    static constexpr auto compAttack    = "compAttack";
    static constexpr auto compRelease   = "compRelease";
    static constexpr auto compAutoRelease = "compAutoRelease";
    static constexpr auto compKnee      = "compKnee";
    static constexpr auto compMakeup    = "compMakeup";
    static constexpr auto compAutoMakeup= "compAutoMakeup";
    static constexpr auto compMix       = "compMix";

    // --- Tape saturator ---
    static constexpr auto satBypass  = "satBypass";
    static constexpr auto satDrive   = "satDrive";
    static constexpr auto satWarmth  = "satWarmth";
    static constexpr auto satMix     = "satMix";

    // --- Stereo widener ---
    static constexpr auto widthBypass    = "widthBypass";
    static constexpr auto widthAmount    = "widthAmount";
    static constexpr auto widthBassMonoHz= "widthBassMonoHz";

    // --- Analog limiter (true-peak safe) ---
    static constexpr auto limBypass  = "limBypass";
    static constexpr auto limCeiling = "limCeiling";
    static constexpr auto limDrive   = "limDrive";
    static constexpr auto limRelease = "limRelease";

    // --- Output ---
    static constexpr auto outputGainDb = "outputGainDb";
    static constexpr auto outputMonoSafety = "outputMonoSafety";
    static constexpr auto masterBypass = "masterBypass";

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using Range = juce::NormalisableRange<float>;
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

        auto dbRange = [] (float lo, float hi) { return Range (lo, hi, 0.01f); };
        auto freqRange = [] (float lo, float hi)
        {
            Range r (lo, hi, 0.01f);
            r.setSkewForCentre (juce::jlimit (lo + 1.0f, hi - 1.0f, std::sqrt (lo * hi)));
            return r;
        };

        // Input
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (inputGainDb, 1), "Input Gain", dbRange (-24.0f, 24.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (inputInvert, 1), "Phase Invert", false));

        // EQ
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (eqBypass, 1), "EQ Bypass", false));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqLowFreq, 1), "Low Freq", freqRange (20.0f, 300.0f), 60.0f, juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqLowBoostDb, 1), "Low Boost", dbRange (0.0f, 12.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqLowCutDb, 1), "Low Tighten (Pultec cut)", dbRange (0.0f, 12.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqLowMidFreq, 1), "Low-Mid Freq", freqRange (150.0f, 2000.0f), 400.0f, juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqLowMidGainDb, 1), "Low-Mid Gain", dbRange (-12.0f, 12.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqLowMidQ, 1), "Low-Mid Q", Range (0.2f, 5.0f, 0.01f), 0.7f));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqHighMidFreq, 1), "High-Mid Freq", freqRange (1000.0f, 8000.0f), 2500.0f, juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqHighMidGainDb, 1), "High-Mid Gain", dbRange (-12.0f, 12.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqHighMidQ, 1), "High-Mid Q", Range (0.2f, 5.0f, 0.01f), 0.7f));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqAirFreq, 1), "Air Freq", freqRange (6000.0f, 20000.0f), 12000.0f, juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqAirGainDb, 1), "Air Gain", dbRange (-12.0f, 12.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (eqIronAmount, 1), "EQ Iron (transformer color)", Range (0.0f, 100.0f, 0.1f), 15.0f, juce::AudioParameterFloatAttributes().withLabel ("%")));

        // Compressor
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (compBypass, 1), "Comp Bypass", false));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compThreshold, 1), "Threshold", dbRange (-40.0f, 0.0f), -18.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compRatio, 1), "Ratio", Range (1.0f, 20.0f, 0.01f, 0.4f), 2.0f, juce::AudioParameterFloatAttributes().withLabel (":1")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compAttack, 1), "Attack", Range (0.1f, 100.0f, 0.01f, 0.3f), 10.0f, juce::AudioParameterFloatAttributes().withLabel ("ms")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compRelease, 1), "Release", Range (10.0f, 1000.0f, 0.1f, 0.3f), 100.0f, juce::AudioParameterFloatAttributes().withLabel ("ms")));
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (compAutoRelease, 1), "Auto Release", true));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compKnee, 1), "Knee", dbRange (0.0f, 12.0f), 6.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compMakeup, 1), "Makeup", dbRange (0.0f, 24.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (compAutoMakeup, 1), "Auto Makeup", true));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (compMix, 1), "Comp Mix", Range (0.0f, 100.0f, 0.1f), 100.0f, juce::AudioParameterFloatAttributes().withLabel ("%")));

        // Saturator
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (satBypass, 1), "Sat Bypass", false));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (satDrive, 1), "Drive", dbRange (0.0f, 24.0f), 6.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (satWarmth, 1), "Warmth", Range (0.0f, 100.0f, 0.1f), 35.0f, juce::AudioParameterFloatAttributes().withLabel ("%")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (satMix, 1), "Sat Mix", Range (0.0f, 100.0f, 0.1f), 100.0f, juce::AudioParameterFloatAttributes().withLabel ("%")));

        // Widener
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (widthBypass, 1), "Width Bypass", false));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (widthAmount, 1), "Width", Range (0.0f, 200.0f, 0.1f), 100.0f, juce::AudioParameterFloatAttributes().withLabel ("%")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (widthBassMonoHz, 1), "Bass Mono Below", freqRange (20.0f, 300.0f), 120.0f, juce::AudioParameterFloatAttributes().withLabel ("Hz")));

        // Limiter
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (limBypass, 1), "Limiter Bypass", false));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (limCeiling, 1), "Ceiling", dbRange (-6.0f, 0.0f), -1.0f, juce::AudioParameterFloatAttributes().withLabel ("dBTP")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (limDrive, 1), "Limiter Drive", dbRange (0.0f, 18.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (limRelease, 1), "Limiter Release", Range (10.0f, 500.0f, 0.1f, 0.4f), 60.0f, juce::AudioParameterFloatAttributes().withLabel ("ms")));

        // Output
        p.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (outputGainDb, 1), "Output Gain", dbRange (-24.0f, 24.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (outputMonoSafety, 1), "Mono Safety Check", false));
        p.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID (masterBypass, 1), "Master Bypass", false));

        return { p.begin(), p.end() };
    }
}
