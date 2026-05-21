#include "Character/Monsters/PTMonsterCharacter.h"

APTMonsterCharacter::APTMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APTMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();

    // TODO: BaseCharacter 초기화 완료 후 InitializeMonster 호출 위치 배치
}

void APTMonsterCharacter::InitializeMonster()
{
    // TODO: 몬스터 데이터 테이블 참조 변수 확인

    // TODO: RowKey 유효성 확인

    // TODO: RowKey 기반 테이블 행 조회 코드 위치 작성

    // TODO: 조회 결과 nullptr 검사 분기 작성

    // TODO: 부모 공통 초기화 함수 호출 위치 작성

    // TODO: 이동속도 적용 코드 위치 작성

    // TODO: 최대 HP / 현재 HP 반영 코드 위치 작성

    // TODO: 공격력 / 방어력 반영 코드 위치 작성

    // TODO: 몬스터 전용 감지 거리 변수 대입 위치 작성

    // TODO: 몬스터 전용 추적 거리 변수 대입 위치 작성

    // TODO: 몬스터 전용 공격 거리 변수 대입 위치 작성

    // TODO: 드롭 보상 데이터 대입 위치 작성

    // TODO: 보스 여부 분기 코드 위치 작성

    // TODO: 초기 상태 설정 호출 위치 작성
}

void APTMonsterCharacter::SetMonsterState(EMonsterState NewState)
{
    CurrentState = NewState;
}

void APTMonsterCharacter::OnDeath()
{

}
