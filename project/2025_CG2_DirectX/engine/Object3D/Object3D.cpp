#include "Object3D.h"
#include "TransformationMatrix.h"
#include "ImGuiManager.h"
#include <numbers>

using namespace MyMath;

void Object3D::Initialize(Object3DBasic* object3DBasic, ModelManager* modelManager, std::string modelFilePath)
{
	object3DBasic_ = object3DBasic;
	modelManager_ = modelManager;

	CreateWVPResource();

	modelData_ = modelManager->GetModelPointer(modelManager->GetModelIndexByFilePath(modelFilePath));

	CreateMTUV();

	camera_ = object3DBasic_->GetDefaultCamera();

	// とりあえず映り込み用のテクスチャをセット
	cubeTextureFilePaths_ = "resources/rostock_laage_airport_4k.dds";

}

void Object3D::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 worldViewProjectionMatrix;

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = worldMatrix.Multiply(viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData_->WVP = modelData_->rootNode_.localMatrix * worldViewProjectionMatrix;
	transformationMatrixData_->World = modelData_->rootNode_.localMatrix * worldMatrix;
	transformationMatrixData_->WorldInverseTranspose = worldMatrix.Inverse().Transpose();

	for (int32_t i = 0; i < modelData_->meshes_.size(); i++) {
		//パラメータからUVTransform用の行列を生成する
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransforms_.at(i).scale);
		uvTransformMatrix = uvTransformMatrix.Multiply(MakeRotateZMatrix(uvTransforms_.at(i).rotate.z));
		uvTransformMatrix = uvTransformMatrix.Multiply(MakeTranslateMatrix(uvTransforms_.at(i).translate));
		materialDatas_.at(i)->uvTransform = uvTransformMatrix;
	}
}

void Object3D::Draw()
{
	// 表示フラグがたっていたらもろもろの描画処理を行う
	if (isDraw_) {

		for (int32_t i = 0; i < modelData_->meshes_.size(); i++) {
			// テクスチャを設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootDescriptorTable(2, modelManager_->GetTextureManager()->GetSrvHandleGPU(textureFilePaths_.at(i)));
			// 映り込み用のテクスチャを設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootDescriptorTable(5, modelManager_->GetTextureManager()->GetSrvHandleGPU(cubeTextureFilePaths_));
			// VBVを設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->IASetVertexBuffers(0, 1, &modelData_->meshes_.at(i).vertexBufferView);
			// wvp用のCBufferの場所を設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
			// マテリアルCBufferの場所を設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResources_.at(i)->GetGPUVirtualAddress());
			// IBVを設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->IASetIndexBuffer(&modelData_->meshes_.at(i).indexBufferView);
			// 描画！ (DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
			object3DBasic_->GetDirectXBasic()->GetCommandList()->DrawIndexedInstanced(UINT(modelData_->meshes_.at(i).vertices.size()), 1, 0, 0, 0);
		}

	}
}

void Object3D::DebugDraw(std::string label)
{
#ifdef USE_IMGUI
	if (ImGui::TreeNode(label.c_str())) {
		ImGui::Checkbox("isDraw", &isDraw_);
		ImGui::SliderFloat3("Scale", reinterpret_cast<float*>(&transform_.scale), -5, 5);
		ImGui::SliderFloat3("Rotate", reinterpret_cast<float*>(&transform_.rotate), -5, 5);
		ImGui::SliderFloat3("Translate", reinterpret_cast<float*>(&transform_.translate), -5, 5);
		for (int32_t i = 0; Material * materialData : materialDatas_) {
			ImGui::Combo(("Lighting[" + std::to_string(i) + "]").c_str(), &materialDatas_.at(i)->enableLighting, "None\0Lambert\0Half Lambert\0\0");
			ImGui::Combo(("Reflection[" + std::to_string(i) + "]").c_str(), &materialDatas_.at(i)->enableReflection, "NoneReflection\0PhongReflection\0BlinnPhongReflection\0\0");
			ImGui::SliderFloat(("Shininess[" + std::to_string(i) + "]").c_str(), &materialDatas_.at(i)->shininess, 0.0f, 10.0f);
			ImGui::DragFloat2(("UVTranslate[" + std::to_string(i) + "]").c_str(), &uvTransforms_.at(i).translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2(("UVScale[" + std::to_string(i) + "]").c_str(), &uvTransforms_.at(i).scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle(("UVRotate[" + std::to_string(i) + "]").c_str(), &uvTransforms_.at(i).rotate.z);
			ImGui::ColorPicker4(("Color[" + std::to_string(i) + "]").c_str(), reinterpret_cast<float*>(&materialDatas_.at(i)->color));
			i++;
		}
		ImGui::TreePop();
	}
#endif
}

void Object3D::SetModelData(std::string modelFilePath)
{

	modelData_ = modelManager_->GetModelPointer(modelManager_->GetModelIndexByFilePath(modelFilePath));

	// ここでマテリアルを作りなおさねばならないかも
	materialDatas_.clear();
	materialResources_.clear();
	uvTransforms_.clear();
	textureFilePaths_.clear();

	CreateMTUV();
}

void Object3D::CreateWVPResource()
{
	// WVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	transformationMatrixResource_ = object3DBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	//単位行列を書き込んでおく
	transformationMatrixData_->WVP = Matrix4x4::MakeIdentity4x4();
	transformationMatrixData_->World = Matrix4x4::MakeIdentity4x4();
	transformationMatrixData_->WorldInverseTranspose = Matrix4x4::MakeIdentity4x4();
}

void Object3D::CreateMTUV()
{
	// メッシュの数だけにマテリアル用のリソースを作る。今回はMaterial1つ分のサイズを用意する
	for (int32_t i = 0; i < modelData_->meshes_.size(); i++) {

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = object3DBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(Material));

		// メッシュ側のdefaultMaterialのデータを読む
		Material* defaultMaterialData = nullptr;
		modelData_->meshes_.at(i).defaultMaterialResource->Map(0, nullptr, reinterpret_cast<void**>(&defaultMaterialData));

		// マテリアルにデータを書き込む
		Material* materialData = nullptr;
		// 書き込むためのアドレスを取得
		materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

		// コピーする
		memcpy(materialData, defaultMaterialData, sizeof(Material));

		// コンテナの末尾にマテリアルを追加
		materialDatas_.push_back(materialData);
		materialResources_.push_back(materialResource);

		// uvTransformを生成
		struct Transform uvTransform = {
	   { 1.0f, 1.0f, 1.0f },
	   { 0.0f, 0.0f, 0.0f },
	   { 0.0f, 0.0f, 0.0f },
		};
		uvTransforms_.push_back(uvTransform);

		// テクスチャをコピー
		textureFilePaths_.push_back(modelData_->meshes_.at(i).defaultTextureFilePath);
	}
}


