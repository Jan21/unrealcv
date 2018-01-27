#include "UnrealCVPrivate.h"
#include "ModelHandler.h"
#include "UE4CVServer.h"
#include "Paths.h"
#include "ObjectPainter.h"
#include "Json.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "IESLoader.h"  //for this you need to add it to search path in project properties: F:\PROJECTS\UNREAL_VXGI\Engine\Source\Editor\UnrealEd\Private\Factories\//
#include "GTCaptureComponent.h"
#include "CaptureManager.h"

TSharedPtr<FJsonObject> ParseJson(FString CompleteFilePath)
{


	FString JsonRaw = "";
	FFileHelper::LoadFileToString(JsonRaw, *CompleteFilePath);


	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonRaw);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{

		return JsonParsed;
	}
	return JsonParsed;
	//TODO handle errors
}

template <typename ObjClass>
ObjClass* LoadObjFromPath(const FName& Path)
{
	if (Path == NAME_None) return NULL;
	//~

	return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), NULL, *Path.ToString()));
}

UStaticMesh* LoadMeshFromPath(const FName& Path)
{
	if (Path == NAME_None) return NULL;
	//~

	return LoadObjFromPath<UStaticMesh>(Path);
}

UMaterial* LoadMatFromPath(const FName& Path)
{
	if (Path == NAME_None) return NULL;
	//~

	return LoadObjFromPath<UMaterial>(Path);

}

//this is used when you have texture saved as .uasset
UTexture2D* LoadTextureFromPath(const FName& Path)
{
	if (Path == NAME_None) return NULL;
	//~

	return LoadObjFromPath<UTexture2D>(Path);

}
UTextureLightProfile* LoadIESFromPath(const FName& Path)
{
	if (Path == NAME_None) return NULL;
	//~

	return LoadObjFromPath<UTextureLightProfile>(Path);

}

void FModelCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	Cmd = FDispatcherDelegate::CreateRaw(this, &FModelCommandHandler::LoadSceneFromJson);
	CommandDispatcher->BindCommand("vget /load [str]", Cmd, "LoadSceneFromJson");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FModelCommandHandler::LogNames);
	CommandDispatcher->BindCommand("vget /logNames [str]", Cmd, "LogNames");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FModelCommandHandler::LoadCameraConfig);
	CommandDispatcher->BindCommand("vget /loadCam [str]", Cmd, "LoadCameraConfig");
}

FExecStatus FModelCommandHandler::LoadSceneFromJson(const TArray<FString>& Path)
{
	// Async version
	FString GameDir = FPaths::GameDir();
	FString CompleteFilePath = GameDir + "Content/JSON/SCENES/" + Path[0] + ".json";	
	TSharedPtr<FJsonObject> JsonParsed=ParseJson(CompleteFilePath);
	TArray <TSharedPtr<FJsonValue>> geometryArr = JsonParsed->GetArrayField("geometry");
	for (int geo = 0; geo != geometryArr.Num(); geo++) {
		TSharedPtr<FJsonObject> GeomInfo = geometryArr[geo]->AsObject();
		FString ModelPathStr = TEXT("/Game/models/" + GeomInfo->GetStringField(TEXT("mesh")));
		TArray <TSharedPtr<FJsonValue>> location = GeomInfo->GetArrayField(TEXT("location"));
		TArray <TSharedPtr<FJsonValue>> rotation = GeomInfo->GetArrayField(TEXT("rotation"));
		FVector NewLocation = FVector(location[0]->AsNumber(), location[1]->AsNumber(), location[2]->AsNumber());
		FRotator NewRotation = FRotator(rotation[0]->AsNumber(),rotation[1]->AsNumber(),rotation[2]->AsNumber());
		TArray <TSharedPtr<FJsonValue>> matArr = GeomInfo->GetArrayField(TEXT("materials"));
		FString Label = GeomInfo->GetStringField(TEXT("label"));
		UStaticMeshComponent* component = LoadModel(ModelPathStr, NewLocation, NewRotation, Label); //returns static mesh component so you can aply mat to it.

		LoadMatForObj(matArr, component);  //set the materials for the static mesh component
		}

	TArray <TSharedPtr<FJsonValue>> lightsArr = JsonParsed->GetArrayField("lights");
	for (int li = 0; li != lightsArr.Num(); li++) {
		TSharedPtr<FJsonObject> liInfo = lightsArr[li]->AsObject();
		CreateLight(liInfo);
	}

	FObjectPainter::Get().Reset();
	return FExecStatus::OK();



}


FExecStatus FModelCommandHandler::LoadCameraConfig(const TArray<FString>& Path){
	UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(0);
	FString GameDir = FPaths::GameDir();
	FString CompleteFilePath = GameDir + "Content/JSON/CAMERA/" + Path[0] + ".json";	
	TSharedPtr<FJsonObject> JsonParsed=ParseJson(CompleteFilePath);
	TArray <TSharedPtr<FJsonValue>> postProcArr = JsonParsed->GetArrayField("postprocesProperties");
	for (int el = 0; el != postProcArr.Num(); el++) {
		TSharedPtr<FJsonObject> propInfo = postProcArr[el]->AsObject();
		FName PropName = FName(*(propInfo->GetStringField(TEXT("propName"))));
		FName OverrideName = FName(*(propInfo->GetStringField(TEXT("overrideName"))));
		double value = propInfo->GetNumberField(TEXT("value"));
		FString mode = "default";
		GTCapturer->SetCapturePostProcessProperty(mode, PropName, value, OverrideName);
	}
	GTCapturer->SetCameraFOV(JsonParsed->GetNumberField(TEXT("fov")));
	return FExecStatus::OK();
}


FExecStatus FModelCommandHandler::LogNames(const TArray<FString>& Args)
{
	ULevel* Level;
	Level = FUE4CVServer::Get().GetPawn()->GetLevel();
		for (AActor* Actor : Level->Actors)
		{
			if (Actor) // Only StaticMeshActor is interesting
			{
				// FString ActorLabel = Actor->GetActorLabel();
				FString ActorLabel = Actor->GetHumanReadableName();
				UE_LOG(LogTemp, Warning, TEXT("Your message %s"),*ActorLabel);
			}
		}

		


	return FExecStatus::OK();
}
FExecStatus FModelCommandHandler::ResetScene(const TArray<FString>& Args)
{
	//you can use console command RestartLevel but if you need to have it in code try the line bellow--have not tried it myself.
	//UGameplayStatics::OpenLevel(FUE4CVServer::Get().GetPawn(), FName(*(GWorld->GetName())), false);
	//
	//ULevel* Level;
	//Level = FUE4CVServer::Get().GetPawn()->GetLevel();
	//	for (AActor* Actor : Level->Actors)
	//	{
	//		if (Actor && Actor->IsA(AStaticMeshActor::StaticClass())) // Only StaticMeshActor is interesting
	//		{
	//			// FString ActorLabel = Actor->GetActorLabel();
	//			 Actor->Destroy();
	//			//UE_LOG(LogTemp, Warning, TEXT("Your message %s"),*ActorLabel);
	//		}
	//	}

		


	return FExecStatus::OK();
}
UTexture2D* LoadTexture2D_FromFile(const FString& FullFilePath, bool& IsValid)
{
	IsValid = false;
	UTexture2D* LoadedT2D = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath))
	{
		return NULL;
	}


	//Create T2D!
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		const TArray<uint8>* UncompressedBGRA = NULL;

		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			//Valid?
			if (!LoadedT2D)
			{
				return NULL;
			}
			//Copy!
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA->GetData(), UncompressedBGRA->Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			//Update!
			LoadedT2D->UpdateResource();
		}
	}

	// Success!
	IsValid = true;
	return LoadedT2D;
}

UTextureLightProfile* LoadIES_FromFile(const FString& FullFilePath, UObject* InParent, bool& IsValid)
{

	IsValid = false;
	//Load From File
	TArray<uint8> RawFileData;
	if (!FFileHelper::LoadFileToArray(RawFileData, *FullFilePath))
	{
		return NULL;
	}
	FIESLoadHelper IESLoadHelper(RawFileData.GetData(), RawFileData.Num());

		if(IESLoadHelper.IsValid())
		{
			TArray<uint8> RAWData;
			float Multiplier = IESLoadHelper.ExtractInRGBA16F(RAWData);

			UTextureLightProfile* Texture = NewObject<UTextureLightProfile>(InParent); //Cast<UTextureLightProfile>( CreateOrOverwriteAsset(UTextureLightProfile::StaticClass(), InParent, Name) );
			if ( Texture )
			{
				Texture->Source.Init(
					IESLoadHelper.GetWidth(),
					IESLoadHelper.GetHeight(),
					/*NumSlices=*/ 1,
					1,
					TSF_RGBA16F,
					RAWData.GetData()
					);

				Texture->AddressX = TA_Clamp;
				Texture->AddressY = TA_Clamp;
				Texture->CompressionSettings = TC_HDR;
				Texture->MipGenSettings = TMGS_NoMipmaps;
				Texture->Brightness = IESLoadHelper.GetBrightness();
				Texture->TextureMultiplier = Multiplier;
			}
			IsValid = true;
			return Texture;
		}

		return NULL;
	
}

FExecStatus FModelCommandHandler::LoadMatForObj(const TArray <TSharedPtr<FJsonValue>>& matArr , UStaticMeshComponent* component)
{

	AActor* actor = component->GetAttachmentRootActor();
	for (int mat = 0; mat != matArr.Num(); mat++) {
		TSharedPtr<FJsonObject> MatInfo = matArr[mat]->AsObject();
		FString PathToMatStr = TEXT("/Game/models/" + MatInfo->GetStringField(TEXT("mat")));
		//load material
		UMaterial* TheMat;
		TheMat = LoadMatFromPath(FName(*PathToMatStr));
		UMaterialInstanceDynamic* TheMaterial_Dyn;
		TheMaterial_Dyn = UMaterialInstanceDynamic::Create(TheMat, actor);
		//set the params
		TArray <TSharedPtr<FJsonValue>> params= MatInfo->GetArrayField(TEXT("params"));
		for (int param = 0; param != params.Num(); param++) {
			TSharedPtr<FJsonObject> parInfo =params[param]->AsObject();
			TheMaterial_Dyn->SetScalarParameterValue( FName(*parInfo->GetStringField(TEXT("name"))) , parInfo->GetNumberField(TEXT("value")));
		}
		//load Textures

		TArray <TSharedPtr<FJsonValue>> textures = MatInfo->GetArrayField(TEXT("textures"));
		for (int tex = 0; tex != textures.Num(); tex++) {
			TSharedPtr<FJsonObject> texInfo = textures[tex]->AsObject();
			FString GameDir = FPaths::GameDir();
			FString PathToLoad = GameDir + "Content/textures/"+texInfo->GetStringField(TEXT("value"));
			bool IsValid;
			UTexture2D* texture = LoadTexture2D_FromFile(PathToLoad,IsValid);
			TheMaterial_Dyn->SetTextureParameterValue(FName(*texInfo->GetStringField(TEXT("name"))), texture);
		}
		TheMaterial_Dyn->TwoSided = 1;
		component->SetMaterial(mat, TheMaterial_Dyn);
	}
	return FExecStatus::OK();
}





UStaticMeshComponent* FModelCommandHandler::LoadModel(const FString& ObjPath,const FVector& Location, const FRotator& Rotation, const FString& ActorLabel)
{
	// Async version
	FString PathStr = ObjPath;

	//static mesh load	
	UStaticMesh* Mesh;
	Mesh = LoadMeshFromPath(FName(*PathStr));

	//new actor
	AStaticMeshActor* newActor = NULL;
	APawn* pawn = FUE4CVServer::Get().GetPawn();
	newActor = pawn->GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation);
	newActor->SetActorLabel(ActorLabel);

	//static mesh component
	UStaticMeshComponent* NewComp;
	NewComp = NewObject<UStaticMeshComponent>(newActor); //newComp is owned by newActor
	NewComp->AttachTo(newActor->GetRootComponent());
	NewComp->SetStaticMesh(Mesh);
	NewComp->RegisterComponent();


	return NewComp;
	// return FExecStatus::InvalidArgument;;


}


APointLight* FModelCommandHandler::CreateLight(const TSharedPtr<FJsonObject>& liInfo)
{

	TArray <TSharedPtr<FJsonValue>> location = liInfo->GetArrayField(TEXT("location"));
	TArray <TSharedPtr<FJsonValue>> rotation = liInfo->GetArrayField(TEXT("rotation"));
	FVector NewLocation = FVector(location[0]->AsNumber(), location[1]->AsNumber(), location[2]->AsNumber());
	FRotator NewRotation = FRotator(rotation[0]->AsNumber(),rotation[1]->AsNumber(),rotation[2]->AsNumber());
	//new actor
	APointLight* newLight = NULL;
	newLight = FUE4CVServer::Get().GetPawn()->GetWorld()->SpawnActor<APointLight>(APointLight::StaticClass(), NewLocation, NewRotation);
	
	ULightComponent* NewLComp=newLight->GetLightComponent();

	bool IsValid;
	FString GameDir = FPaths::GameDir();
	FString filePath = GameDir + "Content/ies/"+liInfo->GetStringField(TEXT("iesfile")); //+texInfo->GetStringField(TEXT("value"));	
	UTextureLightProfile* IesTexture = LoadIES_FromFile(filePath, newLight, IsValid);
	//TODO-jan enable loading ies uassets
	//FString PathToMatStr = TEXT("/Game/HLOD/27");
	//UTextureLightProfile* IesTexture = LoadIESFromPath(FName(*PathToMatStr));
	IesTexture->UpdateResource();
	NewLComp->SetIESTexture(IesTexture);
	NewLComp->SetIntensity(liInfo->GetNumberField(TEXT("intensity")));

	return newLight;
	// return FExecStatus::InvalidArgument;;


}
