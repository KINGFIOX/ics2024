#include <am.h>
#include <klib.h>
#include <x86/x86.h>

#define NR_IRQ 256  // IDT size
#define SEG_KCODE 1
#define SEG_KDATA 2

static Context* (*user_handler)(Event, Context*) = NULL;  //

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

Context* __am_irq_handle(Context* c) {  // call in src/$ISA/trap.S
  if (user_handler) {
    Event ev = {0};
    switch (c->irq) {
      default:
        ev.event = EVENT_ERROR;
        break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

bool cte_init(Context* (*handler)(Event, Context*)) {
  static GateDesc32 idt[NR_IRQ];

  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i++) {
    idt[i] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);  // defined in abstract-machine/am/src/x86/nemu/trap.S
  }

  // ----------------------- interrupts ----------------------------
  idt[32] = GATE32(STS_IG, KSEL(SEG_KCODE), __am_irq0, DPL_KERN);
  // (GateDesc32){(uint32_t)(__am_irq0) & 0xffff, (KSEL(SEG_KCODE)), 0, 0, (STS_IG), 0, (DPL_KERN), 1, (uint32_t)(__am_irq0) >> 16}
  // {
  //    .off_15_0 | Low 16 bits of offset in segment   = (uint32_t)(__am_irq0) & 0xffff,
  //    .cs | Code segment selector                    = (KSEL(SEG_KCODE)),              // ((((1) << 3) | 0x0))
  //    .args | # args, 0 for interrupt/trap gates     = 0,
  //    .rsv1 | Reserved(should be zero I guess)       = 0,
  //    .type | Type(STS_{TG,IG32,TG32})               = (STS_IG),                       // (0xe)
  //    .s | system                                    = 0,
  //    .dpl | Descriptor(meaning new) privilege level = DPL_KERN,                       // (0x0)
  //    .p | Present                                   = 1,                              // (1)
  //    .off_31_16 | High bits of offset in segment    = (uint32_t)(__am_irq0) >> 16,
  // }

  // ---------------------- system call ----------------------------
  idt[0x80] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecsys, DPL_USER);
  idt[0x81] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);  // int $0x81

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return true;
}

Context* kcontext(Area kstack, void (*entry)(void*), void* arg) { return NULL; }

void yield() { asm volatile("int $0x81"); }

bool ienabled() { return false; }

void iset(bool enable) {}
