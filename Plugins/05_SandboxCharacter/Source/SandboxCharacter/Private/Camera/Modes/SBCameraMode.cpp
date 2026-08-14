#include "Camera/Modes/SBCameraMode.h"
#include "Camera/DataAssets/SBCameraModeDefinition.h"

USBCameraMode::USBCameraMode()
{
}

void USBCameraMode::Initialize(USBCameraComponent* InComponent, USBCameraModeDefinition* InDefinition)
{
	CameraComponent = InComponent;
	Definition = InDefinition;
}

bool USBCameraMode::CanEnter_Implementation(const FSBCameraContext& Context) const
{
	return true;
}


int32 USBCameraMode::GetPriority() const
{
	return Definition ? Definition->Priority : 0;
}
