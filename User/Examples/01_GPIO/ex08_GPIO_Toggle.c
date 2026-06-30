/**
 * ============================================================
 * �����ҧ��� 8: GPIO Toggle (��Ѻʶҹ� LED ��� digitalToggle)
 * ============================================================
 *
 * Ἱ�ѧǧ�� (Circuit Diagram):
 *
 *     CH32V003                  LED
 *     --------                  ---
 *     PC0 ----/\/\/\---->|---- GND
 *            220 Ohm
 *
 *     (ǧ������ǡѹ�Ѻ ex01_LED_Blink)
 *
 * ============================================================
 * ���Ѿ����Ҵ��ѧ (Expected Results):
 * - LED ��� PC0 ��о�Ժ��������� 2Hz
 * - ໴ 250ms / �� 250ms ��Ѻ�ѹ
 * - � digitalToggle() ������ͧ㪵����ʶҹ�
 * - ��÷ӧҹ���Ҵ���?? ��� digitalWrite
 * ============================================================
 * ����͹ (WARNINGS):
 * - digitalToggle() �ҹ�һ��غѹ�͹ �����¹����Ѻ
 * - ��÷ӧҹẺ Read-Modify-Write ��໹�е���ԡ
 * - ���� interrupt ������¹�Һ��������ǡѹ �Ҩ�Դ race condition
 * - digitalToggle �ӧҹ�ҡ��� portWrite (��дǡ���)
 * - �������Ѻ�ҹ���ͧ������ҷ���� ����٧
 * - CH32V003 max 8mA �� pin
 * ============================================================
 * แผนผังการทำงาน (Flowchart):
 *
 * flowchart TD
 *     A["SystemCoreClockUpdate()"] --> B["Timer_Init()"]
 *     B --> C["pinMode(PC0, OUTPUT)"]
 *     C --> D["while(1)"]
 *     D --> E["digitalToggle(PC0)"]
 *     E --> F["Delay_Ms(250)"]
 *     F --> D
 * ============================================================
 */

#define CH32V003_PACKAGE  PACKAGE_TSSOP20
#include <SimpleHAL.h>   // ����ź���� SimpleHAL ������

int main(void)           // ����ѹ��ѡ�ͧ�����
{
    SystemCoreClockUpdate();
    Timer_Init();
    pinMode(PC0, PIN_MODE_OUTPUT);  // ��駤Ң� PC0 ��໹������ҵ�ص

    while(1)                 // ǹ�ٻ͹ѹ��
    {
        digitalToggle(PC0);      // ��ѺʶҹТͧ�� PC0 �ѹ��
                                 // ���¡��˹� HIGH  ���¹໹ LOW
                                 // ���¡��˹� LOW   ���¹໹ HIGH
                                 // ���ͧ�ҹ�ҡ͹����͹ digitalWrite

        Delay_Ms(250);       // ˹ǧ���� 250ms
                             // ������Ҥú 1 �ͺ (Toggle + Delay) = 500ms
                             // ���ʶҹФ���ٹҹ 250ms
                             // �ѧ��鹤������ = 2 Hz
    }                        // ����ش while loop ��Ѻ� Toggle ����ա����
}                            // ����ش����ѹ main
