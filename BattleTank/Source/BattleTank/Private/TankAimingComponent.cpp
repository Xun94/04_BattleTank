// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "TankBarrel.h"
#include "TankTurret.h"
#include "Projectile.h"
#include "TankAimingComponent.h"


// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	static ConstructorHelpers::FClassFinder<AProjectile> Proj(TEXT("/Game/Tank/Projectile_BP"));
    if(Proj.Class)
    {
        ProjectileBluePrint = Proj.Class;
    }
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}
void UTankAimingComponent::Initialise(UTankBarrel* BarrelToSet, UTankTurret* TurretToSet)
{
    Barrel = BarrelToSet;
    Turret = TurretToSet;
}

void UTankAimingComponent::BeginPlay()
{
	Super::BeginPlay();
	LastFireTime = FPlatformTime::Seconds();
}

void UTankAimingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if((FPlatformTime::Seconds() - LastFireTime) < ReloadTimeInSeconds)
	{
		FiringState = EFiringState::Reloading;
	}
	else if(IsBarrelMoving())
	{
		FiringState = EFiringState::Aiming;
	}
	else
	{
		FiringState = EFiringState::Locked;
	}
	
}

EFiringState UTankAimingComponent::GetFiringState() const
{
	return FiringState;
}

bool UTankAimingComponent::IsBarrelMoving()
{
	if(!Barrel) { return false; }
	auto BarrelForward = Barrel->GetForwardVector();

	return !BarrelForward.Equals(AimDirection,0.01);
}

void UTankAimingComponent::AimAt(FVector HitLocation)
{
	if(!Barrel) {return;}
	FVector OutLaunchVelocity;
	FVector StartLocation = Barrel->GetSocketLocation(FName("Projectile"));
	bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
		this,
		OutLaunchVelocity,
		StartLocation,
		HitLocation,
		LaunchSpeed,
		false,
		0,
		0,
		ESuggestProjVelocityTraceOption::DoNotTrace
	);

	if(bHaveAimSolution)
	{
		AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveBarrelTowardsTarget();
		MoveTurretTowardsTarget();		
	}
}

void UTankAimingComponent::MoveBarrelTowardsTarget()
{
	if(!Barrel) { return; }
	auto BarrelRotator = Barrel->GetForwardVector().Rotation();
	auto AimAsRotator = AimDirection.Rotation();
	auto DeltaRotator = AimAsRotator - BarrelRotator;
	Barrel->Elevate(DeltaRotator.Pitch);
}

void UTankAimingComponent::MoveTurretTowardsTarget()
{
	if(!Turret) { return; }
	auto TurretRotator = Turret->GetForwardVector().Rotation();
	auto AimAsRotator = AimDirection.Rotation();
	auto DeltaRotator = AimAsRotator - TurretRotator;
	if(DeltaRotator.GetNormalized().Yaw < 180)
	{
		Turret->Rotate(DeltaRotator.GetNormalized().Yaw);
	}
	else
	{
		Turret->Rotate(-DeltaRotator.GetNormalized().Yaw);
	}
	
}

void UTankAimingComponent::Fire()
{
	if(!ensure(Barrel && ProjectileBluePrint)) {return;}
	bool isReloaded = (FPlatformTime::Seconds() - LastFireTime) > ReloadTimeInSeconds;

	if(FiringState != EFiringState::Reloading){
		// Spawn a projectile at the socket location of the barrel.
	auto Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileBluePrint,
		Barrel->GetSocketLocation(FName("Projectile")),
		Barrel->GetSocketRotation(FName("Projectile"))
		);
	Projectile->LaunchProjectile(LaunchSpeed);
	LastFireTime = FPlatformTime::Seconds();

	}
}

