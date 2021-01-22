declare i32 @putchar(i32)
declare i32 @getchar()

define i32 @main() {
	call void @_INIT_GLOBAL_VARS()
	%i = alloca i32
	store i32 65, i32* %i
	%1 = alloca i32
	br label %2

	%3 = load i32, i32* %i
	%4 = icmp sle i32 %3, 90
	%5 = zext i1 %4 to i32
	%6 = icmp ne i32 %5, 0
	br i1 %6, label %7, label %13

	%8 = load i32, i32* %i
	%9 = call i32 @putchar(i32 %8)
	%10 = load i32, i32* %i
	%11 = add i32 %10, 1
	store i32 %11, i32* %i
	%12 = load i32, i32* %i
	br label %2

	ret i32 0
}

define void @_INIT_GLOBAL_VARS(){
	ret void
}