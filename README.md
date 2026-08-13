# GPIO · LCD를 이용한 암호 통신장치 (S32K144)

NXP **S32K144** MCU를 베어메탈(레지스터 직접 제어)로 구현한 마이크로프로세서 텀프로젝트입니다.
GPIO 외부 인터럽트로 장치를 해금하고, LCD에 표시된 난수 중 **약속된 자리수만 골라 키패드로 입력**하면 통신 메시지가 해독되는 구조입니다.

- 팀: 12조 (고경빈, 우희연)
- 보고서: `12조_고경빈_우희연_텀프로젝트.pdf`

---

## 1. 동작 개요

| 단계 | 조건 | 결과 |
|---|---|---|
| 대기 | 전원 ON | LCD에 `push the button`, LED 전부 OFF, 부저 OFF |
| 오답 버튼 | **SW3** 또는 **SW4** | **RED LED** ON + **부저** 작동 |
| 해금 | **SW1 + SW2 + SW5** 모두 누름 | **BLUE LED** ON, LCD에 통신암호(난수) 표시, 7-Segment · 키패드 활성화 |
| 해독 | 정답 4자리 입력 | LCD에 `correct`, **GREEN LED** ON (BLUE OFF) |

통신암호는 난수 전체가 아니라 **정해진 자리수 4개를 정해진 순서로** 읽어야 정답이 됩니다.
현재 규칙은 **7번째 → 3번째 → 1번째 → 10번째** 자리입니다.

---

## 2. 파일 구조

| 파일 | 역할 |
|---|---|
| [main.c](main.c) | 전체 로직. 포트 초기화, LPIT 지연/키스캔 타이머, GPIO 인터럽트, 7-Segment 구동, 암호 판정 |
| [lcd1602A.c](lcd1602A.c) / [lcd1602A.h](lcd1602A.h) | LCD1602A 4-bit 모드 드라이버 (`lcdinit`, `lcdinput`, `lcdcharinput`) |
| [clocks_and_modes.c](clocks_and_modes.c) / [clocks_and_modes.h](clocks_and_modes.h) | SOSC 8MHz → SPLL 160MHz → Normal RUN 80MHz 클럭 설정 (NXP 예제 기반) |
| [ADC.c](ADC.c) / [ADC.h](ADC.h) | ADC0 12-bit 드라이버. 이번 빌드에서는 호출하지 않음 |

`device_registers.h`(S32K144 SDK 헤더)는 프로젝트 외부에서 제공되어야 합니다.

---

## 3. 클럭 및 타이머 구성

### 클럭 트리 — [clocks_and_modes.c](clocks_and_modes.c)

```
SOSC 8 MHz (외부 크리스탈, RANGE=2, EREFS=1)
  └─ SPLL: 8MHz / 1 × 40 / 2 = 160 MHz
       └─ RCCR(SCS=6): DIVCORE=2 → Core  80 MHz
                       DIVBUS=2  → Bus   40 MHz
                       DIVSLOW=3 → Flash 26.6 MHz
```

### LPIT0 — 두 채널을 용도별로 사용

LPIT 클럭 소스는 `PCS=6` (SPLL2_DIV2 = **40 MHz**)입니다.

| 채널 | 용도 | 설정 |
|---|---|---|
| **CH0** | 폴링 방식 지연 | `TVAL = delay × 40` → `delay_us(n)` = n µs |
| **CH1** | 키패드 자동 스캔 | `TVAL = 80,000,000` (2초 주기), `MIER = 0x02`로 인터럽트 enable |

`lcd1602A.c`의 `delay_100ns(n)`도 CH0을 쓰며 `TVAL = n × 4` → n × 100 ns입니다.

### NVIC — `NVIC_init_IRQs()`

| IRQ | 소스 | Priority |
|---|---|---|
| 59 | PORT A (버튼) | 0x0 |
| 49 | LPIT0 채널 1 (키스캔) | 0x0B |

---

## 4. 핀 맵

### 버튼 (PORT A, Falling-edge 인터럽트 `IRQC=10`)

| 스위치 | 핀 | 플래그 | 역할 |
|---|---|---|---|
| SW1 | PTA11 | `k1` | 해금 조건 |
| SW2 | PTA12 | `k2` | 해금 조건 |
| SW3 | PTA13 | `k3` | 오답 → 부저 |
| SW4 | PTA14 | `k4` | 오답 → 부저 |
| SW5 | PTA15 | `k5` | 해금 조건 |

`PTA->PDDR &= ~((1<<11)|...|(1<<15))`로 입력 방향을 명시하고, 핸들러 종료 시 `PCR |= 0x01000000`으로 ISF를 클리어합니다.

### LCD1602A (4-bit 모드) — `BASE = 9`

| 신호 | 핀 |
|---|---|
| D4 ~ D7 | PTD9, PTD10, PTD11, PTD12 |
| EN (BASE+4) | PTD13 |
| RW (BASE+5) | PTD14 |
| RS (BASE+6) | PTD15 |

### RGB LED (Low-active: `PCOR` = ON, `PSOR` = OFF)

| 색 | 핀 | 시점 |
|---|---|---|
| RED | PTD15 | 오답 버튼(SW3/SW4) |
| BLUE | PTD0 | 해금 성공 |
| GREEN | PTD16 | 정답 입력 |

### 부저

| 핀 | 비고 |
|---|---|
| PTD2 | 시작 시 `PCOR`(OFF), 오답 시 `PSOR`(ON) |
| PTD1 | 출력으로 설정만 되어 있고 미사용 |

### 4-digit 7-Segment (동적 구동, PORT C)

| 세그먼트 | A | B | C | D | E | F | G |
|---|---|---|---|---|---|---|---|
| 핀 | PTC1 | PTC2 | PTC3 | PTC12 | PTC13 | PTC15 | PTC7 |

자리 선택: `FND_SEL[4] = {0x0100, 0x0200, 0x0400, 0x0800}` → **PTC8 ~ PTC11**

### 4×3 매트릭스 키패드 (PORT E)

| 방향 | 핀 |
|---|---|
| 열 출력 (스캔) | PTE12, PTE14, PTE15 |
| 행 입력 (풀다운 + 입력필터) | PTE0 ~ PTE3 |

`KeyScan()` 반환값: `0~9`, `11`(`*`), `12`(`#`)

---

## 5. 핵심 로직

### 5-1. 메인 상태 머신

```c
while(1) {
    if(k1 & k2 & k5) {              // SW1·SW2·SW5 모두 눌렸을 때만 해금
        PTD->PCOR |= (1<<0);        // BLUE ON
        lcdinit(); i = 0;
        text_lcd(msg);              // 통신암호 표시
        while(1){ /* 입력·판정 루프 */ }
    }
    else {
        if(k3 | k4) {               // 오답 버튼
            PTD->PCOR |= (1<<15);   // RED ON
            PTD->PSOR |= (1<<2);    // 부저 ON
        }
    }
}
```

버튼 플래그는 인터럽트 핸들러가 세팅하는 래치이므로, 세 개를 순서 상관없이 눌러 모으면 해금됩니다.

### 5-2. 통신암호 생성

```c
unsigned long long k = rand();
char msg[50];
sprintf(msg, "%llu", k);   // 난수를 문자열화해 LCD로 출력
```

### 5-3. 정답 자리수 추출

```c
a1 = (k / 1000000)    % 10;   // 7번째 자리
a2 = (k / 100)        % 10;   // 3번째 자리
a3 = (k / 1)          % 10;   // 1번째 자리
a4 = (k / 1000000000) % 10;   // 10번째 자리

asum = a1*1000 + a2*100 + a3*10 + a4;
asum %= 10000;
```

10의 거듭제곱으로 나눈 뒤 `% 10`으로 원하는 자리만 뽑아, **7 → 3 → 1 → 10번째** 순서로 4자리 정답 `asum`을 조립합니다.

### 5-4. 키 입력 — 타이머 인터럽트 + 센티널 소비

키패드는 메인 루프가 폴링하지 않고 LPIT0 채널 1 인터럽트가 주기적으로 읽어 전역 `key`에 넣습니다.

```c
void LPIT0_Ch1_IRQHandler(void){
    key = KeyScan();
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK;
}
```

메인 루프는 `key`를 소비한 뒤 무효값(`100`)으로 되돌려 같은 입력이 중복 누적되는 것을 막습니다.

```c
if(key < 10) {
    sum = 10 * sum + key;
    sum %= 10000;        // 최근 4자리만 유지하는 시프트 레지스터
    key = 100;           // 소비 표시 — 다음 ISR이 새 값을 채움
}
Seg_out(sum);
```

`sum % 10000` 덕분에 오입력을 해도 계속 입력하면 다시 맞출 수 있습니다.

### 5-5. 정답 판정

```c
if(sum == asum) {
    lcdinit(); i = 0;
    text_lcd("correct");
    PTD->PCOR |= (1<<16);   // GREEN ON
    PTD->PSOR |= (1<<0);    // BLUE OFF
    k1 = 0;                 // 해금 플래그 해제 → 외부 루프 대기 상태로 복귀
    break;
}
```

### 5-6. 7-Segment 동적 구동

`Seg_out(number)`는 천·백·십·일 자리를 분리해 `FND_SEL[j]`로 한 자리씩 선택하고 `seg()`로 세그먼트 패턴을 출력한 뒤 `delay_us(1000)`씩 유지합니다. 4자리를 1 ms 간격으로 돌려 잔상으로 동시 표시되게 합니다.

### 5-7. LCD 문자열 출력

```c
void text_lcd(char *mess){
   while(mess[i] != '\0'){
        if(i % 16 == 0){                // 16자 → 2행으로 이동
            lcdinput(0x80 + 0x40);
            if(i % 32 == 0) lcdinit();  // 32자 → 화면 초기화
            delay_us(500);
        }
        lcdcharinput(mess[i]);
        delay_us(200);
        i++;
   }
}
```

인덱스 `i`가 전역이므로 새 문자열 출력 전에는 반드시 `i = 0`으로 되돌려야 합니다. 코드에서도 `lcdinit(); i=0;` 패턴으로 처리합니다.

---

## 6. 빌드 / 실행 환경

- MCU: NXP S32K144 (Cortex-M4F)
- IDE: S32 Design Studio for ARM
- 외부 하드웨어: LCD1602A, 4-digit 7-Segment, 4×3 매트릭스 키패드, 택트 스위치 5개, 부저, RGB LED
- `stdio.h`의 `sprintf`를 사용하므로 링커에서 newlib / newlib-nano 설정이 필요합니다.

---

## 7. 알려진 제약 · 개선 여지

현재 코드에 남아 있는 한계입니다.

**타이머·인터럽트**
- `LPIT0_Ch1_IRQHandler()`가 `TIF1`이 아니라 **`TIF0`을 클리어**합니다. 채널 1 플래그가 남아 인터럽트가 반복 진입할 수 있습니다.
- `delay_us()`가 매 호출마다 `LPIT0_init()`을 실행하며 `TMR[1].TVAL`을 다시 써서, 키 샘플링 주기가 의도한 2초와 달라질 수 있습니다.
- `KeyScan()`이 ISR 안에서 `delay_us()`(= 같은 LPIT 모듈을 재초기화하는 블로킹 함수)를 호출합니다. 지연 로직과 키스캔을 서로 다른 타이머로 분리하는 편이 안전합니다.

**동작 로직**
- `srand()` 호출이 없어 `rand()`가 매 부팅마다 **같은 난수**를 반환합니다. LPIT 카운터 등을 시드로 넣으면 실행마다 암호가 바뀝니다.
- 오답 시 켠 부저를 **끄는 코드가 없어** 한 번 울리면 계속 울립니다.
- 정답 후 `k1`만 0으로 리셋되고 `k2`, `k5`는 래치로 남습니다. 재시도 시 SW1만 누르면 바로 해금되므로 세 플래그를 함께 초기화해야 합니다. `sum`도 초기화되지 않습니다.

**하드웨어 매핑**
- LCD의 `RS`(PTD15)와 RED LED(PTD15)가 **같은 핀**이라, LED 조작이 LCD 제어선에 영향을 줍니다.
- `PTC->PCOR = 0x7f` / `0xfff`는 비트 0~11만 지우므로 D(PTC12)·F(PTC15) 세그먼트가 남아 자리 간 잔상이 생길 수 있습니다.

**미사용 코드**
- `ADC.c` / `ADC.h`는 빌드에 포함되지만 호출되지 않습니다.
- `PORT_init()`에서 FTM2 클럭을 켜지만 FTM2를 사용하지 않습니다.
- `prekey`, `Delaytime`, `num` 등 이전 버전에서 쓰던 변수가 선언만 남아 있습니다.
- PTD1은 부저용으로 설정만 되고 사용되지 않습니다.
