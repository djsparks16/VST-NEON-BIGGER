#pragma once
#include <JuceHeader.h>

//==============================================================================
// FXChain — global effects, processed after the synth:
//   distortion -> chorus -> phaser -> delay -> reverb -> compressor
//==============================================================================
class FXChain
{
public:
    void attach (juce::AudioProcessorValueTreeState& apvts)
    {
        auto p = [&] (const juce::String& id) { return apvts.getRawParameterValue (id); };

        pDistOn = p ("fxDistOn"); pDistDrive = p ("fxDistDrive"); pDistMix = p ("fxDistMix");
        pChOn = p ("fxChorusOn"); pChRate = p ("fxChorusRate");
        pChDepth = p ("fxChorusDepth"); pChMix = p ("fxChorusMix");
        pPhOn = p ("fxPhaserOn"); pPhRate = p ("fxPhaserRate");
        pPhDepth = p ("fxPhaserDepth"); pPhMix = p ("fxPhaserMix");
        pDlOn = p ("fxDelayOn"); pDlTime = p ("fxDelayTime");
        pDlFb = p ("fxDelayFb"); pDlMix = p ("fxDelayMix");
        pRvOn = p ("fxRevOn"); pRvSize = p ("fxRevSize"); pRvDamp = p ("fxRevDamp");
        pRvWidth = p ("fxRevWidth"); pRvMix = p ("fxRevMix");
        pCpOn = p ("fxCompOn"); pCpThresh = p ("fxCompThresh"); pCpRatio = p ("fxCompRatio");
    }

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        chorus.prepare (spec);
        phaser.prepare (spec);
        reverb.prepare (spec);
        comp.prepare (spec);

        delay.setMaximumDelayInSamples ((int) (spec.sampleRate * 2.5));
        delay.prepare (spec);
        delay.reset();

        comp.setAttack (5.0f);
        comp.setRelease (120.0f);
    }

    void reset()
    {
        chorus.reset();
        phaser.reset();
        reverb.reset();
        comp.reset();
        delay.reset();
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        const int numSamples  = buffer.getNumSamples();
        const int numChannels = juce::jmin (2, buffer.getNumChannels());

        auto block = juce::dsp::AudioBlock<float> (buffer);
        auto ctx   = juce::dsp::ProcessContextReplacing<float> (block);

        // --- Distortion -------------------------------------------------
        if (pDistOn->load() > 0.5f)
        {
            const float drive = 1.0f + 19.0f * pDistDrive->load();
            const float mix   = pDistMix->load();

            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                for (int i = 0; i < numSamples; ++i)
                    d[i] = mix * std::tanh (d[i] * drive) + (1.0f - mix) * d[i];
            }
        }

        // --- Chorus -----------------------------------------------------
        if (pChOn->load() > 0.5f)
        {
            chorus.setRate (pChRate->load());
            chorus.setDepth (pChDepth->load());
            chorus.setCentreDelay (7.0f);
            chorus.setFeedback (0.0f);
            chorus.setMix (pChMix->load());
            chorus.process (ctx);
        }

        // --- Phaser -----------------------------------------------------
        if (pPhOn->load() > 0.5f)
        {
            phaser.setRate (pPhRate->load());
            phaser.setDepth (pPhDepth->load());
            phaser.setCentreFrequency (900.0f);
            phaser.setFeedback (0.3f);
            phaser.setMix (pPhMix->load());
            phaser.process (ctx);
        }

        // --- Delay ------------------------------------------------------
        if (pDlOn->load() > 0.5f)
        {
            const float delaySamples = juce::jlimit (1.0f, (float) (sampleRate * 2.4),
                                                     pDlTime->load() * 0.001f * (float) sampleRate);
            const float fb  = juce::jlimit (0.0f, 0.95f, pDlFb->load());
            const float mix = pDlMix->load();

            delay.setDelay (delaySamples);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                for (int i = 0; i < numSamples; ++i)
                {
                    const float in  = d[i];
                    const float wet = delay.popSample (ch);
                    delay.pushSample (ch, in + wet * fb);
                    d[i] = in + wet * mix;
                }
            }
        }

        // --- Reverb -----------------------------------------------------
        if (pRvOn->load() > 0.5f)
        {
            juce::Reverb::Parameters rp;
            rp.roomSize   = pRvSize->load();
            rp.damping    = pRvDamp->load();
            rp.width      = pRvWidth->load();
            rp.wetLevel   = pRvMix->load();
            rp.dryLevel   = 1.0f - pRvMix->load() * 0.5f;
            rp.freezeMode = 0.0f;
            reverb.setParameters (rp);
            reverb.process (ctx);
        }

        // --- Compressor ---------------------------------------------------
        if (pCpOn->load() > 0.5f)
        {
            comp.setThreshold (pCpThresh->load());
            comp.setRatio (juce::jmax (1.0f, pCpRatio->load()));
            comp.process (ctx);
        }
    }

private:
    double sampleRate = 44100.0;

    juce::dsp::Chorus<float> chorus;
    juce::dsp::Phaser<float> phaser;
    juce::dsp::Reverb reverb;
    juce::dsp::Compressor<float> comp;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay;

    std::atomic<float> *pDistOn {}, *pDistDrive {}, *pDistMix {},
                       *pChOn {}, *pChRate {}, *pChDepth {}, *pChMix {},
                       *pPhOn {}, *pPhRate {}, *pPhDepth {}, *pPhMix {},
                       *pDlOn {}, *pDlTime {}, *pDlFb {}, *pDlMix {},
                       *pRvOn {}, *pRvSize {}, *pRvDamp {}, *pRvWidth {}, *pRvMix {},
                       *pCpOn {}, *pCpThresh {}, *pCpRatio {};
};
