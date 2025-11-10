#include "Object3D.h"
#include "../../Material.h"
#include "../../TransformationMatrix.h"

using namespace MyMath;

void Object3D::Initialize(Object3DBasic* object3DBasic, ModelManager* modelManager, std::string modelFilePath)
{
	object3DBasic_ = object3DBasic;
	modelManager_ = modelManager;

	CreateWVPResource();

	modelData = modelManager->GetModelPointer(modelManager->GetModelIndexByFilePath(modelFilePath));

	camera_ = object3DBasic_->GetDefaultCamera();

}

void Object3D::Update(struct Transform cameraTransform)
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	/*Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WindowsApi::kClientWidth) / float(WindowsApi::kClientHeight), 0.1f, 100.0f);*/
	Matrix4x4 worldViewProjectionMatrix;
	//if (useDebugcamera) {
	//	worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(debugcamera->GetViewMatrix(), debugcamera->GetProjectionMatrix()));
	//} else {
	//	worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	//}

	if (camera_) {
		const Matrix4x4& viewProjectionMatrix = camera_->GetViewProjectionMatrix();
		worldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
	} else {
		worldViewProjectionMatrix = worldMatrix;
	}

	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Object3D::Draw()
{
	if (isDraw_) {
		for (auto& mesh : modelData->meshes) {
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootDescriptorTable(2, modelManager_->GetTextureManager()->GetSrvHandleGPU(mesh.material.textureFilePath));
			//VBVを設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
			//wvp用のCBufferの場所を設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
			//マテリアルCBufferの場所を設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->SetGraphicsRootConstantBufferView(0, mesh.materialResource->GetGPUVirtualAddress());
			//IBVを設定
			object3DBasic_->GetDirectXBasic()->GetCommandList()->IASetIndexBuffer(&mesh.indexBufferView);
			//描画！ (DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
			//if (drawPlane) {
			object3DBasic_->GetDirectXBasic()->GetCommandList()->DrawIndexedInstanced(UINT(mesh.vertices.size()), 1, 0, 0, 0);
			//}
		}
	}
}

void Object3D::SetModelData(std::string modelFilePath)
{
	modelData = modelManager_->GetModelPointer(modelManager_->GetModelIndexByFilePath(modelFilePath));
}

void Object3D::CreateWVPResource()
{
	// WVP用のリソースを作る。TransformationMatrix 1つ分のサイズを用意する
	transformationMatrixResource = object3DBasic_->GetDirectXBasic()->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MyMath::MakeIdentity4x4();
	transformationMatrixData->World = MyMath::MakeIdentity4x4();
}


