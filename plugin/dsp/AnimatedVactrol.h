
#include <algorithm>
#include <cmath>

//Vactrol based on this paper:
//https://www.researchgate.net/publication/271835899_A_Digital_Model_of_the_Buchla_Lowpass-Gate
//Used Claude for implementation

namespace AnimatedLPG
{
    class AnimatedVactrol
    {
    public:
        AnimatedVactrol();
        ~AnimatedVactrol();

        void prepare(double sampleRate);
        void reset();
        void setAttackTime(float seconds);
        void setReleaseTime(float seconds);
        void setControl(float norm);
        void strike(float velocity);
        void releaseGate();
        float tick();
        float getRf() const;

    private:
        static float currentToResistance( float If);
        void updateCoefficients();
        // Eq. 39 component-fit constants.
        static constexpr float kA          = 3.464f;       // Ohm * A^1.4
        static constexpr float kB          = 1136.212f;    // Ohm
        static constexpr float kMinCurrent = 10e-6f;   // 10 uA  -> ~34 MOhm
        static constexpr float kMaxCurrent = 40e-3f;   // 40 mA  -> ~1.45 kOhm
        float mSampleRate                  = 44100.0f;
        float mAttackTime                  = 0.012f;   // 12 ms  (datasheet, brightening)
        float mReleaseTime                 = 0.250f;   // 250 ms (datasheet, dimming)
        float mAttackCoeff                 = 0.0f;
        float mReleaseCoeff                = 0.0f;
        float mCurrent                     = kMinCurrent;
        float mTargetCurrent               = kMinCurrent;

    };
}



