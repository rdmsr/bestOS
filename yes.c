int main()
{
    asm volatile("int $0x42"
                 :
                 : "D"("hello"));

    while (1)
        ;
}
