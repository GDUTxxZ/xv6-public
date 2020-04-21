# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl swtch
swtch:
  # 一开始从esp中弹出两个入参,并保存在eax和edx中,
  movl 4(%esp), %eax
  movl 8(%esp), %edx

  # Save old callee-saved registers
  # 在当前栈上建立一个新的上下文结构
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi

  # Switch stacks
  movl %esp, (%eax)
  movl %edx, %esp

  # Load new callee-saved registers
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  # allocproc 通过把 initproc 的 p->context->eip 设置为 forkret 使得 ret 开始执行 forkret 的代码。
  ret
