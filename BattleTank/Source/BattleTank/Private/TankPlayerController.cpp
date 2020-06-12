// Fill out your copyright notice in the Description page of Project Settings.

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TankPlayerController.h"

void ATankPlayerController::BeginPlay()
{
    Super::BeginPlay();
    auto ControlledTank = GetControlledTank();
    if (!ControlledTank)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController not possessing a tank"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController posessing: %s"), *(ControlledTank->GetName()));   
    }
}

void ATankPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AimTowardsCrosshair();
    //UE_LOG(LogTemp,Warning, TEXT("Player Controller ticking"));

}

ATank* ATankPlayerController::GetControlledTank() const
{
    return Cast<ATank>(GetPawn());
}

void ATankPlayerController::AimTowardsCrosshair()
{
    if(!GetControlledTank()){ return; }

    FVector HitLocation; // Out parameter
    if(GetSightRayHitLocation(HitLocation))
    {
        GetControlledTank()->AimAt(HitLocation);
        //Get world location if linetrace through crosshair
        //If it hits the landscape
            //Tell controlled tank to aim at this point
    }
}

bool ATankPlayerController::GetSightRayHitLocation(FVector& HitLocation) const
{
    int32 ViewportsizeX, ViewportsizeY;
    GetViewportSize(ViewportsizeX, ViewportsizeY);
    auto ScreenLocation = FVector2D(ViewportsizeX * CrosshairXLocation, ViewportsizeY * CrosshairYLocation);
    FVector LookDirection;
    if(GetLookDirection(ScreenLocation,LookDirection))
    {
        //UE_LOG(LogTemp, Warning, TEXT("Look direction : %s"), *LookDirection.ToString());
        return GetLookVectorHitLocation(LookDirection, HitLocation);
    }

    return true;
}

bool ATankPlayerController::GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const
{
    FVector CameraWorldLocation;
    return DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, CameraWorldLocation, LookDirection);
}

bool ATankPlayerController::GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const
{
    FHitResult HitResult;
    auto StartLocation = PlayerCameraManager->GetCameraLocation();
    auto EndLocation = StartLocation + (LookDirection * LineTraceRange);
    if(GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility)
        )
    {
       HitLocation = HitResult.Location;
       return true; 
    }
    HitLocation = FVector(0.0);
    return false;
}