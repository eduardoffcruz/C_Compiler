char global_char = '\000';
char global_char1 = '\377';
char global_char2 = '\4';
char global_char3 = '\44';
char global_char4 = '\344';

char global_a = 'A';
short global_b = 'B';
int global_c = 'C';
double global_d = 4.0;

int main(void) {
    // Printing, variable declaration, assignment, unary minus.
    char a = 'D';
    short b = 'E';
    int c = 'F';
    double d = 4.0;
    int temp;

    temp = -(-2147483648);
    temp = -2147483648;

    // A
    temp = a;
    a = global_a;
    putchar(-(-(-(-a))));
    a = temp;

    // B
    temp = b;
    b = global_b;
    putchar(b);
    b = temp;

    // C
    temp = c;
    c = global_c;
    putchar(-(-c));
    c = temp;

    // D E F
    putchar(a);
    putchar(b);
    putchar(c);

    // D B B D
    a = a;
    b = -'B';
    c = -a;
    putchar(a);
    putchar(-b);
    putchar(-(-(-(-(-(-'B'))))));
    putchar(-(-(-(-'B'))));
    // putchar(-(-(-(-'B'))));
    putchar(-(-(-(-(-(-(-c)))))));

    putchar('\n');
    return -(-(-c));
}
