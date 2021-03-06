#pragma once

#include <complex>
#include <vector>


struct SOceanParams
{
	Math::Vector2 windDirection;
	float windSpeed;

	SOceanParams (float angle = 0.0f, float speed = 31.0f) :
		windDirection (std::cosf (angle), std::sinf (angle)),
		windSpeed (speed)
	{
	}
};


// most calculation is base on [2001, Jerry Tessendorf] Simulating Ocean Water.
class CWaveSimulator
{
public:
    CWaveSimulator (SOceanParams params);

    void Update (float currentTime);
    
    const void *GetDisplacementData () { return (void *)mDisplacementData.data (); }
    const void *GetNormalData () { return (void *)mNormalData.data (); }
    int GetDataSize () { return mFFTSize; }
    
    void DebugSave (const char *path);

private:
	// these data won't change, so we only need to do the computing once
	struct CacheData
	{
		Math::Vector2 k; // k vector
		Math::Vector2 kn; // nomalized k vector
        std::complex<float> expKDotX; // exp (dot (kvector,  horizontal position x))
		float w; // wave frequency
		std::complex<float> h0; // fourier amplitude of a wave height field
		std::complex<float> h0cn; // conj(h0(-k))
	};

private:
	void InitDataLUT ();
	Math::Vector2 ComputeK (int x, int y);
	std::complex<float> ComputeExpKDotX (const Math::Vector2 &k, int x, int y);
	float ComputeWaveFrequency (const Math::Vector2 &k);
	std::complex<float> ComputeFourierAmplitude0 (const Math::Vector2 &k);
	float ComputePhillipsSpectrum (const Math::Vector2 &k);

	// Fourier amplitude/displacement/normal of the wave field realization at time t
	void ComputeFourierField (int x, int y, float t); 

	// reverse fast fourier transform
	void FFT2D (std::vector<std::complex<float>> &v);
	void FFT1D (std::vector<float> &real, std::vector<float> &imag);

	inline int IndexLookup (int x, int y) { return y * mFFTSize + x; }
    inline int PowNeg1 (int n) { static int pow[2] = { 1, -1 }; return pow[n & 1]; }

	void NormalizeData ();
    
    void DebugSaveData (const char *path, const std::vector<Math::Vector3> &v, int index);

private:
	SOceanParams mEnvironmentParams;

	const int mFFTSize = 64; // N/M, must be power of 2
	const float mWorldSize = 100.0f; // Lx/Lz, meters
	const float mPhillipsSpectrumConstant = 1.0f;


	const float g = 9.81f; // Gravity constant
	const float pi = (float)M_PI; // 
	const int mFFTSizeHalf = mFFTSize / 2;
	const int mFFTSizeLog = (int)std::log2 (mFFTSize); // log2(fftsize), use for fft calculation
	const float mMinimalWaveScale = mWorldSize * 0.001f; // waves length way smaller than the world size should be suppressed


	std::vector<CacheData> mDataLUT; // pre-computed data we need to use in every frame
	std::vector<std::complex<float>> mHeightField; // fourier amplitude of the wave field realization at time t
    std::vector<std::complex<float>> mDisplacementFieldX;
    std::vector<std::complex<float>> mDisplacementFieldZ;
    std::vector<std::complex<float>> mNormalFieldX;
    std::vector<std::complex<float>> mNormalFieldZ;


	std::vector<Math::Vector3> mDisplacementData;
	std::vector<Math::Vector3> mNormalData;
};
