// effects.scd
s.waitForBoot {
	~currentEffect = { |sig| sig }; // Default effect: passthrough
	~currentEffectId = 0;
	~pitchShiftRatio = 1.0;
	~reverbMix = 0.5;
	~reverbRoom = 0.8;
	~distortionVal = 1.0;
	~volume = 1;
	~setCurrentEffect = {
		switch(~currentEffectId)
		{ 0 } { ~currentEffect = { |sig| 0 } }
		{ 1 } { ~currentEffect = { |sig| sig * ~volume } }
		{ 2 } { ~currentEffect = { |sig| PitchShift.ar(sig, pitchRatio: ~pitchShiftRatio, mul: ~volume) } }
		{ 3 } { ~currentEffect = { |sig| FreeVerb.ar(sig, mix: ~reverbMix, room: ~reverbRoom, mul: ~volume) } }
		{ 4 } { ~currentEffect = { |sig| sig.tanh * ~distortionVal * ~volume} }
		{ 5 } { // Darth Vader effect
			~currentEffect = { |sig|
				var out = PitchShift.ar(sig, pitchRatio: 0.75);
				out = BPeakEQ.ar(out, 200,db: 2);
				out = CombN.ar(out,0.02, 0.015, 0.05, add: out);
				out = out * ~volume;
				out
			}
		}
		{ 6 } { // Vocoder effect
			~currentEffect = { |sig|
				var out = PitchShift.ar(sig,pitchRatio: 0.8, mul: 0.2, add:0.4* sig);
				out = CombN.ar(PitchShift.ar(sig,pitchRatio: 0.8 + 0.4.rand, mul: 0.3, add: out),0.05, 0.05.rand, 0.05);
				out = CombN.ar(PitchShift.ar(sig,pitchRatio: 0.7 + 0.6.rand, mul: 0.3, add: out),0.05, 0.05.rand, 0.05);
				out = CombN.ar(PitchShift.ar(sig,pitchRatio: 0.5 + 1.rand, mul: 0.2, add: out),0.05, 0.05.rand, 0.05);
				out = CombN.ar(PitchShift.ar(sig,pitchRatio: 0.3 + 1.7.rand, mul: 0.2, add: out),0.05, 0.05.rand, 0.05);
				out = CombN.ar(PitchShift.ar(sig,pitchRatio: 0.8 + 0.2.rand, mul: 0.2, add: out),0.05, 0.05.rand, 0.05);
				BPeakEQ.ar(out, 200,db: 2);
			}
		}
		{ 7 } { // Robot effect (Bit crushing + Ring modulation)
			~currentEffect = { |sig|
				var crushed = Latch.ar(sig, Impulse.ar(~bitRate)); // Lower sampling rate
				var ringMod = sig * SinOsc.ar(~ringFreq, 0, ~ringModAmt);
				crushed = crushed.round((2 ** (~bitDepth - 1)) / (2 ** ~bitDepth)); // Reduce bit depth
				(crushed + ringMod) * ~volume;
			}
		}
		{ 8 } { // Echo/Delay effect
			~currentEffect = { |sig|
				CombN.ar(sig, delaytime: ~delayTime, decaytime: ~decayTime, mul: ~volume)
			}
		}
		{ 9 } { // Telephone effect
			~currentEffect = { |sig|
				sig = BPF.ar(sig, freq: 1000, rq: 0.9); // Bandpass for a "tinny" sound
				sig = sig * ~volume;
				sig
			}
		}
		{ "Unknown effect: %".format(~currentEffectId).postln };
		Ndef(\effect).source = { |sig| ~currentEffect.value(SoundIn.ar(0)) };
		Ndef(\effect).play;
	};

	OSCdef(\setEffect, { |msg|
		~currentEffectId = msg[1];
		~setCurrentEffect.value;
	}, '/effect'); // Listen for effect selection

	OSCdef(\setParam, { |msg|
		var id = msg[1];
		var val = msg[2];
		switch(id)

		{ 1 } { ~volume = val }
		{ 2 } { ~pitchShiftRatio = val }
		{ 3 } { ~reverbMix = val }
		{ 4 } { ~reverbRoom = val }
		{ 5 } { ~distortionVal = val }
		{ 6 } { ~vocoderFreq = val } // Frequency of the vocoder carrier wave
		{ 7 } { ~vocoderAmp = val } // Amplitude of the vocoder carrier wave
		{ 8 } { ~vocoderBands = val } // Number of bands in the vocoder
		{ 9 } { ~bitRate = val } // Bit rate for the bit-crushing effect
		{ 10 } { ~bitDepth = val } // Bit depth for the bit-crushing effect
		{ 11 } { ~ringFreq = val } // Frequency for ring modulation
		{ 12 } { ~ringModAmt = val } // Amplitude of the ring modulation
		{ 13 } { ~delayTime = val } // Delay time for the echo effect
		{ 14 } { ~decayTime = val }; // Decay time for the echo effect

		~setCurrentEffect.value;
	}, '/param'); // Listen for parameter updates

	Ndef(\effect).source = { ~currentEffect.value(SoundIn.ar(0)) };
	Ndef(\effect).play;
};

