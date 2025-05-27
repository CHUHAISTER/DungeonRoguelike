// Fill out your copyright notice in the Description page of Project Settings.
#include "DRPlayerController.h"
#include "MathCombat/MathManager.h"
#include "Blueprint/UserWidget.h"

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
    MathWidgetInstance = CreateWidget<UMathWidget>(this, MathWidgetClass);
    MathWidgetInstance->SetTask(MathManager);   
    MathWidgetInstance->AddToViewport();


}

