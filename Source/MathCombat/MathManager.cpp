// Fill out your copyright notice in the Description page of Project Settings.


#include "MathManager.h"

FText UMathManager::GenerateTask()
{
    int32 First = FMath::RandRange(1, 20);
    int32 Second = FMath::RandRange(1, 20);

    
    FString TaskString = FString::Printf(TEXT("%d + %d = ?"), First, Second);
    LastCorrectAnswer = First + Second;
    FText Ex = FText::FromString(TaskString);
    Examples.Add(Ex);
    return Ex;
}



