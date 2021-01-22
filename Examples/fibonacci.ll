declare i32 @putchar(i32)
declare i32 @getchar()

define i32 @factorial(i32) {
	%n = alloca i32
	store i32 %0, i32* %n
	%2 = load i32, i32* %n
	%3 = icmp eq i32 %2, 0
	%4 = zext i1 %3 to i32
	%5 = icmp ne i32 %4, 0
	br i1 %5, label %6, label %8

	ret i32 1
	br label %15

	%9 = load i32, i32* %n
	%10 = load i32, i32* %n
	%11 = sub i32 %10, 1
	%12 = call i32 @factorial(i32 %11)
	%13 = mul i32 %9, %12
	ret i32 %13
	br label %15

	ret i32 0
}

define i32 @fibbonacci(i32) {
	%n = alloca i32
	store i32 %0, i32* %n
	%2 = load i32, i32* %n
	%3 = icmp eq i32 %2, 0
	%4 = zext i1 %3 to i32
	%5 = icmp ne i32 %4, 0
	br i1 %5, label %6, label %8

	ret i32 0
	br label %25

	%9 = load i32, i32* %n
	%10 = icmp eq i32 %9, 1
	%11 = zext i1 %10 to i32
	%12 = icmp ne i32 %11, 0
	br i1 %12, label %13, label %15

	ret i32 1
	br label %24

	%16 = load i32, i32* %n
	%17 = sub i32 %16, 1
	%18 = call i32 @fibbonacci(i32 %17)
	%19 = load i32, i32* %n
	%20 = sub i32 %19, 2
	%21 = call i32 @fibbonacci(i32 %20)
	%22 = add i32 %18, %21
	ret i32 %22
	br label %24

	br label %25

	ret i32 0
}

define i32 @main() {
	call void @_INIT_GLOBAL_VARS()
	%n = alloca i32
	store i32 5, i32* %n
	%i = alloca i32
	store i32 0, i32* %i
	%1 = call i32 @putchar(i32 10)
	%2 = load i32, i32* %i
	%3 = call i32 @fibbonacci(i32 %2)
	%4 = add i32 48, %3
	%5 = call i32 @putchar(i32 %4)
	%6 = alloca i32
	br label %7

	%8 = load i32, i32* %i
	%9 = load i32, i32* %n
	%10 = icmp slt i32 %8, %9
	%11 = zext i1 %10 to i32
	%12 = icmp ne i32 %11, 0
	br i1 %12, label %13, label %22

	%14 = load i32, i32* %i
	%15 = add i32 %14, 1
	store i32 %15, i32* %i
	%16 = load i32, i32* %i
	%17 = call i32 @putchar(i32 45)
	%18 = load i32, i32* %i
	%19 = call i32 @fibbonacci(i32 %18)
	%20 = add i32 %19, 48
	%21 = call i32 @putchar(i32 %20)
	br label %7

	ret i32 0
}

define void @_INIT_GLOBAL_VARS(){
	ret void
}