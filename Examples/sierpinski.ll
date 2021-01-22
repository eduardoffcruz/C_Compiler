declare i32 @putchar(i32)
declare i32 @getchar()

define i32 @main() {
	call void @_INIT_GLOBAL_VARS()
	%i = alloca i32
	store i32 0, i32* %i
	%j = alloca i32
	%1 = alloca i32
	br label %2

	%3 = load i32, i32* %i
	%4 = icmp slt i32 %3, 32
	%5 = zext i1 %4 to i32
	%6 = icmp ne i32 %5, 0
	br i1 %6, label %7, label %33

	store i32 0, i32* %j
	%8 = load i32, i32* %j
	%9 = alloca i32
	br label %10

	%11 = load i32, i32* %j
	%12 = icmp slt i32 %11, 32
	%13 = zext i1 %12 to i32
	%14 = icmp ne i32 %13, 0
	br i1 %14, label %15, label %28

	%16 = load i32, i32* %i
	%17 = load i32, i32* %j
	%18 = and i32 %16, %17
	%19 = icmp ne i32 %18, 0
	br i1 %19, label %20, label %22

	%21 = call i32 @putchar(i32 32)
	br label %24

	%23 = call i32 @putchar(i32 43)
	br label %24

	%25 = load i32, i32* %j
	%26 = add i32 %25, 1
	store i32 %26, i32* %j
	%27 = load i32, i32* %j
	br label %10

	%29 = load i32, i32* %i
	%30 = add i32 %29, 1
	store i32 %30, i32* %i
	%31 = load i32, i32* %i
	%32 = call i32 @putchar(i32 10)
	br label %2

	ret i32 0
}

define void @_INIT_GLOBAL_VARS(){
	ret void
}