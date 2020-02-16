/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             SynthLevelControlTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Synthesiser with level control.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

//==============================================================================
class MainContentComponent   : public AudioAppComponent
{
public:
    MainContentComponent()
    {
        levelSlider_1.setRange (0.0, 0.25);
        levelSlider_1.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
        levelLabel_1.setText ("Noise Level L", dontSendNotification);

        addAndMakeVisible (levelSlider_1);
        addAndMakeVisible (levelLabel_1);
        
        levelSlider_2.setRange (0.0, 0.25);
        levelSlider_2.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
        levelLabel_2.setText ("Noise Level R", dontSendNotification);

        addAndMakeVisible (levelSlider_2);
        addAndMakeVisible (levelLabel_2);

        setSize (600, 100);
        setAudioChannels (0, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int, double) override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        auto level_1 = (float) levelSlider_1.getValue();
        auto levelScale_1 = level_1 * 2.0f;
        
        auto level_2 = (float) levelSlider_2.getValue();
        auto levelScale_2 = level_2 * 2.0f;

        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto* buffer = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
            
            auto level = 0.f;
            auto levelScale = 0.f;
            
            if (channel == 0)
            {
                level = level_1;
                levelScale = levelScale_1;
            }
            else
            {
                level = level_2;
                levelScale = levelScale_2;
            }
            
            for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
                buffer[sample] = random.nextFloat() * levelScale - level;
        }
    }

    void releaseResources() override {}

    void resized() override
    {
        levelLabel_1 .setBounds (10, 10, 90, 20);
        levelSlider_1.setBounds (100, 10, getWidth() - 110, 20);
        
        levelLabel_2.setBounds (10, 100, 90, 20);
        levelSlider_2.setBounds (100, 100, getWidth() - 110, 20);
    }

private:
    Random random;
    Slider levelSlider_1;
    Label  levelLabel_1;
    Slider levelSlider_2;
    Label  levelLabel_2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
