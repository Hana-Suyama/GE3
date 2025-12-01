#include "XAudio2Basic.h"
#include <cassert>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "../utility/StringUtility.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

void XAudio2Basic::Initialize()
{
	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
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
	xAudio2.Reset();
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

	////ファイル入力ストリームのインスタンス
	//std::ifstream file;
	//// .wavファイルをバイナリモードで開く
	//file.open(filename, std::ios_base::binary);
	////ファイルオープン失敗を検出する
	//assert(file.is_open());

	////RIFFヘッダーの読み込み
	//RiffHeader riff;
	//file.read((char*)&riff, sizeof(riff));
	////ファイルがRIFFかチェック
	//if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
	//	assert(0);
	//}
	////タイプがWAVEかチェック
	//if (strncmp(riff.type, "WAVE", 4) != 0) {
	//	assert(0);
	//}

	////Formatチャンクの読み込み
	//FormatChunk format = {};
	////チャンクヘッダーの確認
	//file.read((char*)&format, sizeof(ChunkHeader));
	//if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
	//	assert(0);
	//}

	////チャンク本体の読み込み
	//assert(format.chunk.size <= sizeof(format.fmt));
	//file.read((char*)&format.fmt, format.chunk.size);

	////Dataチャンクの読み込み
	//ChunkHeader data;
	//file.read((char*)&data, sizeof(data));
	////Junkチャンクを検出した場合
	//if (strncmp(data.id, "JUNK", 4) == 0) {
	//	//読み取り位置をJUNKチャンクの終わりまで進める
	//	file.seekg(data.size, std::ios_base::cur);
	//	//再読み込み
	//	file.read((char*)&data, sizeof(data));
	//}

	//if (strncmp(data.id, "data", 4) != 0) {
	//	assert(0);
	//}

	////Dataチャンクのデータ部(波形データ)の読み込み
	//char* pBuffer = new char[data.size];
	//file.read(pBuffer, data.size);

	////Waveファイルを閉じる
	//file.close();

	////returnする為の音声データ
	//SoundData soundData = {};

	//soundData.wfex = format.fmt;
	//soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	//soundData.bufferSize = data.size;

	//return soundData;

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

	SoundPlayWave(xAudio2.Get(), soundData);
}

