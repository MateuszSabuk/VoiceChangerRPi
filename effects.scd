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
		{1}{~volume = val}
		{2}{~pitchShiftRatio = val}
		{3}{~reverbMix = val}
		{4}{~reverbRoom = val}
		{5}{~distortionVal = val};
		~setCurrentEffect.value;
	}, '/param'); // Listen for parameter updates

	Ndef(\effect).source = { ~currentEffect.value(SoundIn.ar(0)) };
	Ndef(\effect).play;
};