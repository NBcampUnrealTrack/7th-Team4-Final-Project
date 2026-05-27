#include "PTCommonButtonBase.h"
#include "CommonActionWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PTCommonButtonBase)

// 에디터 프리뷰 및 위젯 생성 전 초기화
void UPTCommonButtonBase::NativePreConstruct()
{
    Super::NativePreConstruct();

    UpdateButtonStyle();
    RefreshButtonText();
}

// 할당된 입력 액션(키 바인딩 등)이 변경될 때 호출
void UPTCommonButtonBase::UpdateInputActionWidget()
{
    Super::UpdateInputActionWidget();

    UpdateButtonStyle();
    RefreshButtonText();
}

// 수동으로 버튼 텍스트 설정 (비어있으면 자동 입력 텍스트 사용)
void UPTCommonButtonBase::SetButtonText(const FText& InText)
{
    bOverride_ButtonText = InText.IsEmpty();
    ButtonText = InText;
    RefreshButtonText();
}

// 상황에 맞게 텍스트 갱신 (입력 액션 텍스트 vs 수동 텍스트)
void UPTCommonButtonBase::RefreshButtonText()
{
    // 수동 텍스트가 없거나 덮어쓰기 상태일 경우
    if (bOverride_ButtonText || ButtonText.IsEmpty())
    {
        if (InputActionWidget)
        {
            // 입력 액션 위젯에 할당된 텍스트(예: "Spacebar") 가져오기
            const FText ActionDisplayText = InputActionWidget->GetDisplayText();
            if (!ActionDisplayText.IsEmpty())
            {
                UpdateButtonText(ActionDisplayText);
                return;
            }
        }
    }

    // 수동으로 설정한 텍스트 표시
    UpdateButtonText(ButtonText);
}

// 입력 디바이스(키보드/마우스 <-> 게임패드) 변경 시 호출
void UPTCommonButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
    Super::OnInputMethodChanged(CurrentInputType);

    // 변경된 디바이스에 맞춰 버튼 스타일 갱신
    UpdateButtonStyle();
}


