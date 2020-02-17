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

 name:             AudioThumbnailTutorial
 version:          4.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files and displays a thumbnail.

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

// example implementation of the Exercise 3
//==============================================================================
class SimpleThumbnailComponent : public Component,
                                 private ChangeListener
{
public:
    SimpleThumbnailComponent (int sourceSamplesPerThumbnailSample,
                              AudioFormatManager& formatManager,
                              AudioThumbnailCache& cache)
       : thumbnail (sourceSamplesPerThumbnailSample, formatManager, cache)
    {
        thumbnail.addChangeListener (this);
    }

    void setFile (const File& file)
    {
        thumbnail.setSource (new FileInputSource (file));
    }

    void paint (Graphics& g) override
    {
        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded (g);
        else
            paintIfFileLoaded (g);
    }

    void paintIfNoFileLoaded (Graphics& g)
    {
        g.fillAll (Colours::white);
        g.setColour (Colours::darkgrey);
        g.drawFittedText ("No File Loaded", getLocalBounds(), Justification::centred, 1);
    }

    void paintIfFileLoaded (Graphics& g)
    {
        g.fillAll(Colours::white);

        g.setColour (Colours::red);
        thumbnail.drawChannels (g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
            thumbnailChanged();
    }

private:
    void thumbnailChanged()
    {
        repaint();
    }

    AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleThumbnailComponent)
};

//------------------------------------------------------------------------------

class SimplePositionOverlay : public Component,
                              private Timer
{
public:
    SimplePositionOverlay (AudioTransportSource& transportSourceToUse)
       : transportSource (transportSourceToUse)
    {
        startTimer (40);
    }

    void paint (Graphics& g) override
    {
        auto duration = (float) transportSource.getLengthInSeconds();

        if (duration > 0.0)
        {
            auto audioPosition = (float) transportSource.getCurrentPosition();
            auto drawPosition = (audioPosition / duration) * getWidth();

            g.setColour (Colours::green);
            g.drawLine (drawPosition, 0.0f, drawPosition, (float) getHeight(), 2.0f);
        }
    }

    void mouseDown (const MouseEvent& event) override
    {
        auto duration = transportSource.getLengthInSeconds();

        if (duration > 0.0)
        {
            auto clickPosition = event.position.x;
            auto audioPosition = (clickPosition / getWidth()) * duration;

            transportSource.setPosition (audioPosition);
        }
    }

private:
    void timerCallback() override
    {
        repaint();
    }

    AudioTransportSource& transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplePositionOverlay)
};

//------------------------------------------------------------------------------

class MainContentComponent   : public AudioAppComponent,
                               public ChangeListener
{
public:
    MainContentComponent()
      : state (Stopped),
        thumbnailCache (5),
        thumbnailComp (512, formatManager, thumbnailCache),
        positionOverlay (transportSource)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (TextButton::buttonColourId, Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (TextButton::buttonColourId, Colours::red);
        stopButton.setEnabled (false);

        addAndMakeVisible (&thumbnailComp);
        addAndMakeVisible (&positionOverlay);

        setSize (600, 400);

        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);

        setAudioChannels (2, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
            bufferToFill.clearActiveBufferRegion();
        else
            transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        playButton.setBounds (10, 40, getWidth() - 20, 20);
        stopButton.setBounds (10, 70, getWidth() - 20, 20);

        Rectangle<int> thumbnailBounds (10, 100, getWidth() - 20, getHeight() - 120);
        thumbnailComp.setBounds (thumbnailBounds);
        positionOverlay.setBounds (thumbnailBounds);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
            transportSourceChanged();
    }

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:
                    playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:
                    stopButton.setEnabled (true);
                    break;

                case Stopping:
                    transportSource.stop();
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }
    }

    void transportSourceChanged()
    {
        if (transportSource.isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }

    void openButtonClicked()
    {
        FileChooser chooser ("Select a Wave file to play...",
                             {},
                             "*.wav;*.mp3");

        if (chooser.browseForFileToOpen())
        {
            File file = chooser.getResult();

            if (auto* reader = formatManager.createReaderFor (file))
            {
                std::unique_ptr<AudioFormatReaderSource> newSource (new AudioFormatReaderSource (reader, true));
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                playButton.setEnabled (true);
                thumbnailComp.setFile (file);
                readerSource.reset (newSource.release());
            }
        }
    }

    void playButtonClicked()
    {
        changeState (Starting);
    }

    void stopButtonClicked()
    {
        changeState (Stopping);
    }

    //==========================================================================
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;
    AudioThumbnailCache thumbnailCache;
    SimpleThumbnailComponent thumbnailComp;
    SimplePositionOverlay positionOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
