// Fill out your copyright notice in the Description page of Project Settings.


#include "MathWidget.h"


void UMathWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_0) Button_0->OnClicked.AddDynamic(this, &UMathWidget::OnButton0Clicked);
	if (Button_1) Button_1->OnClicked.AddDynamic(this, &UMathWidget::OnButton1Clicked);
	if (Button_2) Button_2->OnClicked.AddDynamic(this, &UMathWidget::OnButton2Clicked);
	if (Button_3) Button_3->OnClicked.AddDynamic(this, &UMathWidget::OnButton3Clicked);
	if (Button_4) Button_4->OnClicked.AddDynamic(this, &UMathWidget::OnButton4Clicked);
	if (Button_5) Button_5->OnClicked.AddDynamic(this, &UMathWidget::OnButton5Clicked);
	if (Button_6) Button_6->OnClicked.AddDynamic(this, &UMathWidget::OnButton6Clicked);
	if (Button_7) Button_7->OnClicked.AddDynamic(this, &UMathWidget::OnButton7Clicked);
	if (Button_8) Button_8->OnClicked.AddDynamic(this, &UMathWidget::OnButton8Clicked);
	if (Button_9) Button_9->OnClicked.AddDynamic(this, &UMathWidget::OnButton9Clicked);

	if (Button_Enter) Button_Enter->OnClicked.AddDynamic(this, &UMathWidget::Click_Enter);
	if (Button_Delete) Button_Delete->OnClicked.AddDynamic(this, &UMathWidget::OnDeletePressed);

}




void UMathWidget::SetTask(FText Task)
{
	if (TaskText)
	{
		TaskText->SetText(Task);
	}
}

void UMathWidget::Click_Enter()
{
	if (ResultString.Len() > 0)
	{
		RemoveFromParent();

		if (APlayerController* PC = GetOwningPlayer())
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
			PC->SetPause(false);
		}
	}
};

void UMathWidget::OnNumberPressed(FString Number)
{
	ResultString += Number;
	UpdateResultText();
}

void UMathWidget::OnDeletePressed()
{
	if (ResultString.Len() > 0)
	{
		ResultString.LeftChopInline(1);
		UpdateResultText();
	}
}

void UMathWidget::UpdateResultText()
{
	if (Result)
	{
		Result->SetText(FText::FromString(ResultString));
	}
}
