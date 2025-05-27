// Fill out your copyright notice in the Description page of Project Settings.
#include "DRPlayerController.h"
#include "MathCombat/MathManager.h"
#include "Blueprint/UserWidget.h"
#include "DungeonRoguelikeCharacter.h"

void ADRPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController() && UHealthBarClass)
    {
        HealthBarWidget = CreateWidget<UHealthBar>(this, UHealthBarClass);
        if (HealthBarWidget)
        {
            HealthBarWidget->AddToViewport();
            UE_LOG(LogTemp, Warning, TEXT("HealthBarWidget successfully generated!"));
        }
    }
    MathManager = NewObject<UMathManager>();
}

void ADRPlayerController::StartTaskMenu()
{
    bCanShoot = false;
    MathWidgetInstance = CreateWidget<UMathWidget>(this, MathWidgetClass);
    MathWidgetInstance->OnAnswerFinished.AddDynamic(this, &ADRPlayerController::OnMathWidgetAnswer);
    MathWidgetInstance->SetTask(MathManager);
    MathWidgetInstance->AddToViewport();


}
void  ADRPlayerController::OnMathWidgetAnswer(bool bIsCorrect)
{
    if (bIsCorrect)
    {
        ADungeonRoguelikeCharacter* MyChar = Cast<ADungeonRoguelikeCharacter>(GetPawn());
        FVector MuzzleLocation = MyChar->GetActorLocation() + MyChar->GetActorForwardVector() * 100.f + FVector(0, 0, 50.f);
        FRotator MuzzleRotation = GetControlRotation();

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(MyChar->ProjectileClass, MuzzleLocation, MuzzleRotation, Params);
        if (Projectile)
        {
            FVector LaunchDir = MuzzleRotation.Vector();
            Projectile->FireInDirection(LaunchDir);
        }
    }
}
void ADRPlayerController::SaveExamplesToFile()
{
    FString DateTimeStr = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M:%S"));
    FString Header = FString::Printf(TEXT("Дата і час збереження: %s"), *DateTimeStr);
    UE_LOG(LogTemp, Warning, TEXT("SAVE!"));

    TArray<FString> Lines;
    Lines.Add(Header);
    Lines.Append(MathManager->Examples);

    FString SaveDir = FPaths::ProjectSavedDir();
    FString FilePath = SaveDir / TEXT("ExamplesLog.txt");

    FFileHelper::SaveStringArrayToFile(Lines, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8);

}
