<div align="center">

# 🔐 암호 통신장치 (Cipher Communication Device)

**NXP S32K144 MCU를 베어메탈로 제어해 만든, 약속된 자리수만 아는 사람만 해독할 수 있는 임베디드 암호 입력 장치**

![Platform](https://img.shields.io/badge/Platform-NXP%20S32K144-blue?style=flat-square)
![Core](https://img.shields.io/badge/Core-ARM%20Cortex--M4F-orange?style=flat-square)
![Language](https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c)
![IDE](https://img.shields.io/badge/IDE-S32%20Design%20Studio-green?style=flat-square)
![License](https://img.shields.io/badge/License-BSD--3--Clause%20(partial)-lightgrey?style=flat-square)

</div>

---

## 📖 프로젝트 개요

수업에서 따로따로 배운 주변장치들(GPIO 인터럽트 · 7-Segment · 매트릭스 키패드 · Character LCD · 타이머)을 **하나의 시나리오 안에서 함께 동작시키는 것**이 목표였습니다.

단순히 "버튼 누르면 LED 켜기" 수준을 넘기 위해 **암호 통신**이라는 컨셉을 잡았습니다.

> LCD에 표시되는 것은 매번 달라지는 난수입니다.
> 하지만 **정해진 자리수 4개를, 정해진 순서로** 읽어야 정답이 됩니다.
> 규칙(7번째 → 3번째 → 1번째 → 10번째)을 아는 사람만 해독할 수 있습니다.

SDK의 드라이버 API를 쓰지 않고 **레지스터를 직접 제어**(베어메탈)하여, 클럭 트리 구성부터 인터럽트 벡터 연결까지 직접 다뤘습니다.

<br>

## ✨ 주요 기능

### 1️⃣ 다중 버튼 조합 인증 (GPIO 외부 인터럽트)

`SW1 + SW2 + SW5`를 모두 눌러야 장치가 해금됩니다. 폴링이 아닌 **PORT A Falling-edge 인터럽트**로 구현해 메인 루프를 점유하지 않습니다.

### 2️⃣ 오답 피드백 (부저 + RGB LED)

해금 조건이 아닌 `SW3` / `SW4`를 누르면 **RED LED와 부저**가 작동해 즉시 오답임을 알립니다.

### 3️⃣ 난수 암호 생성 및 자리수 추출

`rand()`로 생성한 난수를 `sprintf()`로 문자열화해 LCD에 출력하고, 그중 **10의 거듭제곱 나눗셈**으로 약속된 4개 자리만 뽑아 정답을 조립합니다.

### 4️⃣ 타이머 인터럽트 기반 키패드 입력

4×3 매트릭스 키패드를 **LPIT0 채널 1 인터럽트**가 주기적으로 스캔합니다. 입력값은 4-digit 7-Segment에 실시간으로 동적 표시되며, 정답 일치 시 LCD에 `correct`가 출력되고 **GREEN LED**로 전환됩니다.

<br>

## 🚀 시작하기 (Getting Started)

### 사전 요구 사항

| 구분 | 항목 |
|---|---|
| **하드웨어** | S32K144 EVB, LCD1602A, 4-digit 7-Segment, 4×3 매트릭스 키패드, 택트 스위치 5개, 부저, RGB LED |
| **소프트웨어** | [S32 Design Studio for ARM](https://www.nxp.com/design/software/development-software/s32-design-studio-ide) |
| **디버거** | OpenSDA (EVB 내장) |

> ⚠️ **이 저장소에는 애플리케이션 소스만 포함되어 있습니다.**
> `device_registers.h`, startup 코드, 링커 스크립트 등 **NXP가 배포하는 SDK 파일은 라이선스 문제로 포함하지 않았습니다.** 아래 절차대로 새 프로젝트를 생성해 주세요.

### 설치 및 빌드

**1. 저장소 클론**

```bash
git clone https://github.com/hiyeonwhy/microprossor.git
```

**2. S32DS에서 빈 프로젝트 생성**

```
File → New → S32DS Application Project
  ├─ Processor : S32K144
  └─ Toolchain : ARM GCC
```

이 과정에서 아래 파일들이 자동 생성됩니다. 애플리케이션 코드가 의존하는 필수 요소입니다.

| 자동 생성 파일 | 역할 |
|---|---|
| `device_registers.h`, `S32K144.h` | 레지스터·매크로 정의 |
| `startup_S32K144.S` | **벡터 테이블** (`PORTA_IRQHandler`, `LPIT0_Ch1_IRQHandler` 연결) |
| `system_S32K144.c` | `SystemInit()` |
| `S32K144_*_flash.ld` | 링커 스크립트 |

**3. 소스 복사**

클론한 저장소의 `.c` / `.h` 파일 7개를 생성된 프로젝트의 `src/` 폴더에 복사합니다.

**4. 빌드 설정 확인** ⚙️

`sprintf()`에서 `%llu`(long long) 포맷을 사용하므로, **newlib-nano의 축소 printf를 사용하면 LCD에 값이 출력되지 않습니다.**

```
Properties → C/C++ Build → Settings → Standard S32DS C Linker → Support
  └─ "Use nano formatting" 체크 해제
```

**5. 빌드 및 플래시**

```
Project → Build Project  (Ctrl + B)
Run → Debug As → S32DS Application
```

<br>

## 🎮 사용법 (Usage)

### 동작 시나리오

| 단계 | 사용자 입력 | 장치 반응 |
|:---:|---|---|
| **1. 대기** | 전원 ON | LCD: `push the button` / LED·부저 OFF |
| **2. 오답** | `SW3` 또는 `SW4` | 🔴 RED LED ON + 🔊 부저 작동 |
| **3. 해금** | `SW1` + `SW2` + `SW5` | 🔵 BLUE LED ON / LCD에 난수 표시 / 키패드·7-Seg 활성화 |
| **4. 해독** | 키패드로 정답 4자리 입력 | LCD: `correct` / 🟢 GREEN LED ON |

### 암호 해독 규칙

LCD에 아래와 같이 표시되었다고 가정합니다.

```
┌──────────────────┐
│ 1804289383       │   ← rand()로 생성된 통신암호
└──────────────────┘
   ↑     ↑ ↑     ↑
  10번  7번 3번  1번   ← 오른쪽에서 세는 자리
```

`1804289383`을 오른쪽부터 세면 1번째=`3`, 3번째=`3`, 7번째=`4`, 10번째=`1` 입니다.
정답은 **7 → 3 → 1 → 10** 순서이므로 키패드에 `4331`을 입력하면 됩니다.

### 핵심 코드

**자리수 추출** — [main.c](main.c)

```c
a1 = (k / 1000000)    % 10;   // 7번째 자리
a2 = (k / 100)        % 10;   // 3번째 자리
a3 = (k / 1)          % 10;   // 1번째 자리
a4 = (k / 1000000000) % 10;   // 10번째 자리

asum = a1*1000 + a2*100 + a3*10 + a4;
asum %= 10000;
```

**타이머 인터럽트 키스캔** — 메인 루프가 키패드를 폴링하지 않습니다.

```c
void LPIT0_Ch1_IRQHandler(void){
    key = KeyScan();
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK;
}
```

**시프트 레지스터 방식 입력 누적** — 오입력해도 계속 입력하면 다시 맞출 수 있습니다.

```c
if(key < 10) {
    sum = 10 * sum + key;
    sum %= 10000;        // 최근 4자리만 유지
    key = 100;           // 소비 표시 (중복 누적 방지)
}
```

<br>

## 📁 폴더 구조

```
microprossor/
├── main.c                 # 🎯 메인 로직
│                          #    · PORT_init()          포트/핀 먹스 초기화
│                          #    · LPIT0_init()         타이머 (CH0 지연 / CH1 키스캔)
│                          #    · PORTA_IRQHandler()   버튼 인터럽트
│                          #    · LPIT0_Ch1_IRQHandler() 키패드 자동 스캔
│                          #    · Seg_out() / seg()    7-Segment 동적 구동
│                          #    · KeyScan()            4×3 매트릭스 스캔
│                          #    · main()               상태 머신 + 암호 판정
│
├── lcd1602A.c / .h        # 📺 LCD1602A 4-bit 모드 드라이버
├── clocks_and_modes.c/.h  # ⏱️ SOSC 8MHz → SPLL 160MHz → Core 80MHz
├── ADC.c / .h             # 📊 ADC0 12-bit 드라이버 (현재 미사용)
│
├── 12조_..._텀프로젝트.pdf  # 📄 프로젝트 보고서
└── README.md
```

### 시스템 구성

**클럭 트리**

```
SOSC 8 MHz (외부 크리스탈)
  └─ SPLL: 8MHz / 1 × 40 / 2 = 160 MHz
       └─ Core 80 MHz │ Bus 40 MHz │ Flash 26.6 MHz
```

**LPIT0 채널 할당** (클럭 소스 = SPLL2_DIV2 = 40 MHz)

| 채널 | 용도 | 설정 |
|:---:|---|---|
| CH0 | 폴링 지연 | `TVAL = delay × 40` → `delay_us(n)` = n µs |
| CH1 | 키패드 자동 스캔 | `TVAL = 80,000,000` (2초 주기), IRQ 49 |

**인터럽트**

| IRQ | 소스 | Priority |
|:---:|---|:---:|
| 59 | PORT A (버튼) | 0x0 |
| 49 | LPIT0 채널 1 (키스캔) | 0x0B |

<br>

## 🔌 핀 맵

<details>
<summary><b>전체 핀 배치 펼치기</b></summary>

<br>

**버튼** — PORT A, Falling-edge (`IRQC = 10`)

| 스위치 | 핀 | 플래그 | 역할 |
|:---:|:---:|:---:|---|
| SW1 | PTA11 | `k1` | 해금 조건 |
| SW2 | PTA12 | `k2` | 해금 조건 |
| SW3 | PTA13 | `k3` | 오답 → 부저 |
| SW4 | PTA14 | `k4` | 오답 → 부저 |
| SW5 | PTA15 | `k5` | 해금 조건 |

**LCD1602A** — 4-bit 모드, `BASE = 9`

| 신호 | 핀 |
|---|---|
| D4 ~ D7 | PTD9 ~ PTD12 |
| EN | PTD13 |
| RW | PTD14 |
| RS | PTD15 |

**RGB LED / 부저** — LED는 Low-active (`PCOR` = ON)

| 소자 | 핀 | 시점 |
|---|:---:|---|
| 🔴 RED | PTD15 | 오답 버튼 |
| 🔵 BLUE | PTD0 | 해금 성공 |
| 🟢 GREEN | PTD16 | 정답 입력 |
| 🔊 부저 | PTD2 | 오답 시 ON |

**4-digit 7-Segment** — PORT C

| 세그먼트 | A | B | C | D | E | F | G |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| 핀 | PTC1 | PTC2 | PTC3 | PTC12 | PTC13 | PTC15 | PTC7 |

자리 선택: `FND_SEL[4] = {0x0100, 0x0200, 0x0400, 0x0800}` → PTC8 ~ PTC11

**4×3 매트릭스 키패드** — PORT E

| 방향 | 핀 |
|---|---|
| 열 출력 (스캔) | PTE12, PTE14, PTE15 |
| 행 입력 (풀다운 + 입력필터) | PTE0 ~ PTE3 |

`KeyScan()` 반환값: `0~9`, `11`(`*`), `12`(`#`)

</details>

<br>

<div align="center">
<sub>Made with ⚡ by 12조 · 고경빈 · 우희연</sub>
</div>
