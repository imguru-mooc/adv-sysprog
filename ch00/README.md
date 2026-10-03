# 고급 Linux System Programming II · Chapter 0 — 과정 소개 · 실습 환경 준비

| File | 내용 |
|---|---|
| `lecture.html` | 강의 자료. 따라 하기 실습, 빈칸 채우기 · 괄호 넣기 (자동 확인, 답은 Browser 에 저장), 정답 보기, 인쇄 |
| `animation.html` | 애니메이션 2종. Unit 마다 하나, 단계별로 직접 조작 |
| `worksheet.html` | 개념 워크시트 20문항 (Unit 당 10문항). 채점 → 제출 → 오답 정리 캡처 |
| `slide.pdf` | 강의 Slide |
| `src/` | 예제 Source. `cd src && make` 로 전체 빌드 |

| Unit | 주제 | 영상 | 애니메이션 |
|---|---|---|---|
| 0.1 | 과정 소개 | 6분 | C0-1 Process 단면도로 보는 8 Chapter |
| 0.2 | 실습 환경 준비 | 10분 | C0-2 환경 점검 시뮬레이터 |

## 사용 방법

1. 영상을 보고 `lecture.html` 을 따라 합니다. 출력이 다르면 각 Unit 의 "오류 해결" 표를 확인합니다.
2. 🎬 표시가 나오면 `animation.html` 을 직접 조작해 봅니다.
3. Unit 을 마치면 `worksheet.html` 의 해당 탭 10문항을 풉니다.

예제 받기 (VM 에서):

```bash
git clone https://github.com/imguru-mooc/adv-sysprog.git ~/lsp2      # 처음 한 번
cd ~/lsp2 && git pull          # 이후 업데이트
cd ~/lsp2/ch00/src && make
```

온라인으로 보기: https://imguru-mooc.github.io/adv-sysprog/ch00/lecture.html
