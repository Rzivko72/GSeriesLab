#pragma once
#include <JuceHeader.h>
#include <array>
class GSeriesLabProcessor final : public juce::AudioProcessor {
public:
 GSeriesLabProcessor();
 void prepareToPlay(double sampleRate, int samplesPerBlock) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "GSeries Lab"; }
 bool acceptsMidi() const override { return false; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 0.0; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return {}; }
 void changeProgramName(int, const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*, int) override;
 juce::AudioProcessorValueTreeState state;
 std::atomic<float> gainReductionDb {0.f};
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
 double fs=44100.0;
 float envelope=0.f, slowEnvelope=0.f;
 std::array<float,2> hpX{}, hpY{};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GSeriesLabProcessor)
};
