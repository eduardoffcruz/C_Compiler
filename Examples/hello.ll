declare i32 @putchar(i32)
declare i32 @getchar()

define i32 @main() {
	call void @_INIT_GLOBAL_VARS()
	%a = alloca i32
	store i32 72, i32* %a
	%b = alloca i32
	store i32 101, i32* %b
	%1 = load i32, i32* %a
	%2 = call i32 @putchar(i32 %1)
	%3 = load i32, i32* %b
	%4 = call i32 @putchar(i32 %3)
	%5 = call i32 @putchar(i32 108)
	%6 = call i32 @putchar(i32 108)
	%7 = call i32 @putchar(i32 111)
	%c = alloca i32
	store i32 32, i32* %c
	%d = alloca i32
	store i32 87, i32* %d
	%8 = load i32, i32* %c
	%9 = call i32 @putchar(i32 %8)
	%10 = load i32, i32* %d
	%11 = call i32 @putchar(i32 %10)
	%e = alloca i32
	store i32 111, i32* %e
	%12 = load i32, i32* %e
	%13 = call i32 @putchar(i32 %12)
	%14 = call i32 @putchar(i32 114)
	%15 = call i32 @putchar(i32 108)
	%16 = call i32 @putchar(i32 100)
	%f = alloca i32
	store i32 10, i32* %f
	%17 = load i32, i32* %f
	%18 = call i32 @putchar(i32 %17)
	ret i32 0
}

define void @_INIT_GLOBAL_VARS(){
	ret void
}