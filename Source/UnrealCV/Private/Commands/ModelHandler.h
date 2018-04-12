#pragma once
#include "CommandDispatcher.h"
#include "CommandHandler.h"

class FModelCommandHandler : public FCommandHandler
{
private:
	AStaticMeshActor* LoadedScene;
	UStaticMeshComponent* LoadedSceneComp;
public:
	FModelCommandHandler() : FCommandHandler()
	{}
	void RegisterCommands();

	FExecStatus LoadSceneFromJson(const TArray<FString>& Path);
	FExecStatus LoadCameraConfig(const TArray<FString>& Path);
	FExecStatus LogNames(const TArray<FString>& Args);
	FExecStatus ResetScene(const TArray<FString>& Args);
	FExecStatus LoadMatForObj(const TArray <TSharedPtr<FJsonValue>>& matArr, UStaticMeshComponent* component);
	UStaticMeshComponent* LoadModel(const FString& ObjPath, const FVector& Location, const FRotator& Rotation, const FString& ActorLabel);

	APointLight * CreateLight(const TSharedPtr<FJsonObject>& liInfo);



};
