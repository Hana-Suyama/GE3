#pragma once

#include <xaudio2.h>
#include <cstdint>
#include <wrl.h>
#include <unordered_map>
#include <fstream>

#pragma comment(lib, "xaudio2.lib")

//チャンクヘッダ
struct ChunkHeader {
	char id[4];	//チャンク毎のID
	int32_t size;	//チャンクのサイズ
};

//RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;	//"RIFF"
	char type[4];	//"WAVE"
};

//FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;	//"fmt"
	WAVEFORMATEX fmt;	//波形フォーマット
};

//音声データ
struct SoundData {
	WAVEFORMATEX wfex;	//波形フォーマット
	std::vector<BYTE> buffer;	//バッファ
	std::string filePath;	//ファイルパス
};

class XAudio2Basic
{
public:

	void Initialize();

	void Finalize();

	SoundData SoundLoadFile(const char* filename);

	void SoundUnload(SoundData* soundData);

	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);

	void LoadSound(const std::string& filePath);

	void PlayAudio(const std::string& filePath);

	//音声関連
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

private:
	
	IXAudio2MasteringVoice* masterVoice;

	std::unordered_map<std::string, SoundData> soundDatas_;

	//サウンドデータの読み込み上限数
	const uint32_t kSoundMax_ = 128;

};

