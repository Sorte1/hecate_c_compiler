
int add_int(int a, int b);

extern void inspect(void *);

int main() {
    int x = add_int(1001, 2002);
    inspect(&x);
}

int add_int(int a, int b) {
    return a + b;
}
