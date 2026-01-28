#include "XAudio2Basic.h"
#include <cassert>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "StringUtility.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

void XAudio2Basic::Initialize()
{
	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	//マスターボイスを生成
	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(hr));

	// Windows Media Foundationの初期化(ローカルファイル版)
	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));

}

void XAudio2Basic::Finalize()
{
	for (std::pair<std::string, SoundData> soundData : soundDatas_) {
		SoundUnload(&soundData.second);
	}
	soundDatas_.clear();

	// Windows Media Foundationの終了
	HRESULT hr = MFShutdown();
	assert(SUCCEEDED(hr));

	//XAudio2解放
	xAudio2_.Reset();
}

SoundData XAudio2Basic::SoundLoadFile(const char* filename) {

	// フルパスをワイド文字列に変換
	std::wstring filePathW = StringUtility::ConvertString(filename);
	HRESULT hr;

	// SourceReader作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pReader;
	hr = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(hr));

	// PCM形式にフォーマット指定する
	Microsoft::WRL::ComPtr<IMFMediaType> pPCMType;
	MFCreateMediaType(&pPCMType);
	pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(hr));

	// 実際にセットされたメディアタイプを取得する
	Microsoft::WRL::ComPtr<IMFMediaType> pOutType;
	pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

	// Waveフォーマットを取得する
	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

	// コンテナに格納する音声データ
	SoundData soundData = {};
	soundData.wfex = *waveFormat;

	// 生成したWaveフォーマットを解放
	CoTaskMemFree(waveFormat);

	// PCMデータのバッファを構築
	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;
		// サンプルを読み込む
		hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		// ストリームの末尾に達したら抜ける
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
		
		if (pSample) {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
			// サンプルに含まれるサウンドデータのバッファを一つなぎにして取得
			pSample->ConvertToContiguousBuffer(&pBuffer);

			BYTE* pData = nullptr;	//データ読み取り用ポインタ
			DWORD maxLength = 0, currentLength = 0;
			// バッファ読み込み用にロック
			pBuffer->Lock(&pData, &maxLength, &currentLength);
			// バッファの末尾にデータを追加
			soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);
			pBuffer->Unlock();
		}
	}

	return soundData;
}

void XAudio2Basic::SoundUnload(SoundData* soundData)
{
	soundData->buffer.clear();
	soundData->wfex = {};
}

void XAudio2Basic::SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{
	HRESULT result;

	//波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(soundData.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

void XAudio2Basic::LoadSound(const std::string& filePath)
{
	// 既に同じファイルが読み込まれていたらreturn
	if (soundDatas_.contains(filePath)) {
		return;
	}

	// 最大読み込み数を超えていたらassert
	assert(kSoundMax_ > soundDatas_.size());

	SoundData& newSound = soundDatas_[filePath];

	newSound = SoundLoadFile(filePath.c_str());

}

void XAudio2Basic::PlayAudio(const std::string& filePath)
{
	SoundData& soundData = soundDatas_.at(filePath);

	SoundPlayWave(xAudio2_.Get(), soundData);
}

