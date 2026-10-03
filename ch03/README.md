# 고급 Linux System Programming II · Chapter 3 — Page Cache와 Advanced File I/O

| File | 내용 |
|---|---|
| `lecture.html` | 강의 자료. 따라 하기 실습, 빈칸 채우기 · 괄호 넣기 (자동 확인, 답은 Browser 에 저장), 정답 보기, 인쇄 |
| `animation.html` | 애니메이션 5종. Lecture 마다 하나, 단계별로 직접 조작 |
| `worksheet.html` | 개념 워크시트 50문항 (Lecture 당 10문항). 채점 → 제출 → 오답 정리 캡처 |
| `slide.pdf` | 강의 Slide |
| `src/` | 예제 Source. `cd src && make` 로 전체 빌드 |

| Lecture | 주제 | 영상 | 애니메이션 |
|---|---|---|---|
| L11 | read/write 뒤에서 실제로 일어나는 일 | 17분 | C3-1 read() / write() Path |
| L12 | VFS와 file_operations | 16분 | C3-2 VFS Dispatch |
| L13 | Page Cache | 17분 | C3-3 Page Cache Simulator |
| L14 | Buffered I/O / Direct I/O / mmap 비교 | 17분 | C3-4 Buffered / Direct / mmap 경로 비교 |
| L15 | Writeback, fsync 그리고 데이터 영속성 | 18분 | C3-5 Writeback과 fsync |

## 사용 방법

1. 영상을 보고 `lecture.html` 을 따라 합니다. 출력이 다르면 각 Lecture 의 "오류 해결" 표를 확인합니다.
2. 🎬 표시가 나오면 `animation.html` 을 직접 조작해 봅니다.
3. Lecture 을 마치면 `worksheet.html` 의 해당 탭 10문항을 풉니다.

예제 받기 (VM 에서):

```bash
git clone https://github.com/imguru-mooc/adv-sysprog.git ~/lsp2      # 처음 한 번
cd ~/lsp2 && git pull          # 이후 업데이트
cd ~/lsp2/ch03/src && make
```

온라인으로 보기: https://imguru-mooc.github.io/adv-sysprog/ch03/lecture.html
