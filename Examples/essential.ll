declare i32 @putchar(i32)
declare i32 @getchar()

@global_char = global i32 0
@global_char1 = global i32 0
@global_char2 = global i32 0
@global_char3 = global i32 0
@global_char4 = global i32 0
@global_a = global i32 0
@global_b = global i32 0
@global_c = global i32 0
@global_d = global double 0.0
define i32 @main() {
	call void @_INIT_GLOBAL_VARS()
	%a = alloca i32
	store i32 68, i32* %a
	%b = alloca i32
	store i32 69, i32* %b
	%c = alloca i32
	store i32 70, i32* %c
	%d = alloca double
	store double 4.0, double* %d
	%temp = alloca i32
	%1 = sub i32 0, 2147483648
	%2 = sub i32 0, %1
	store i32 %2, i32* %temp
	%3 = load i32, i32* %temp
	%4 = sub i32 0, 2147483648
	store i32 %4, i32* %temp
	%5 = load i32, i32* %temp
	%6 = load i32, i32* %a
	store i32 %6, i32* %temp
	%7 = load i32, i32* %temp
	%8 = load i32, i32* @global_a
	store i32 %8, i32* %a
	%9 = load i32, i32* %a
	%10 = load i32, i32* %a
	%11 = sub i32 0, %10
	%12 = sub i32 0, %11
	%13 = sub i32 0, %12
	%14 = sub i32 0, %13
	%15 = call i32 @putchar(i32 %14)
	%16 = load i32, i32* %temp
	store i32 %16, i32* %a
	%17 = load i32, i32* %a
	%18 = load i32, i32* %b
	store i32 %18, i32* %temp
	%19 = load i32, i32* %temp
	%20 = load i32, i32* @global_b
	store i32 %20, i32* %b
	%21 = load i32, i32* %b
	%22 = load i32, i32* %b
	%23 = call i32 @putchar(i32 %22)
	%24 = load i32, i32* %temp
	store i32 %24, i32* %b
	%25 = load i32, i32* %b
	%26 = load i32, i32* %c
	store i32 %26, i32* %temp
	%27 = load i32, i32* %temp
	%28 = load i32, i32* @global_c
	store i32 %28, i32* %c
	%29 = load i32, i32* %c
	%30 = load i32, i32* %c
	%31 = sub i32 0, %30
	%32 = sub i32 0, %31
	%33 = call i32 @putchar(i32 %32)
	%34 = load i32, i32* %temp
	store i32 %34, i32* %c
	%35 = load i32, i32* %c
	%36 = load i32, i32* %a
	%37 = call i32 @putchar(i32 %36)
	%38 = load i32, i32* %b
	%39 = call i32 @putchar(i32 %38)
	%40 = load i32, i32* %c
	%41 = call i32 @putchar(i32 %40)
	%42 = load i32, i32* %a
	store i32 %42, i32* %a
	%43 = load i32, i32* %a
	%44 = sub i32 0, 66
	store i32 %44, i32* %b
	%45 = load i32, i32* %b
	%46 = load i32, i32* %a
	%47 = sub i32 0, %46
	store i32 %47, i32* %c
	%48 = load i32, i32* %c
	%49 = load i32, i32* %a
	%50 = call i32 @putchar(i32 %49)
	%51 = load i32, i32* %b
	%52 = sub i32 0, %51
	%53 = call i32 @putchar(i32 %52)
	%54 = sub i32 0, 66
	%55 = sub i32 0, %54
	%56 = sub i32 0, %55
	%57 = sub i32 0, %56
	%58 = sub i32 0, %57
	%59 = sub i32 0, %58
	%60 = call i32 @putchar(i32 %59)
	%61 = sub i32 0, 66
	%62 = sub i32 0, %61
	%63 = sub i32 0, %62
	%64 = sub i32 0, %63
	%65 = call i32 @putchar(i32 %64)
	%66 = load i32, i32* %c
	%67 = sub i32 0, %66
	%68 = sub i32 0, %67
	%69 = sub i32 0, %68
	%70 = sub i32 0, %69
	%71 = sub i32 0, %70
	%72 = sub i32 0, %71
	%73 = sub i32 0, %72
	%74 = call i32 @putchar(i32 %73)
	%75 = call i32 @putchar(i32 10)
	%76 = load i32, i32* %c
	%77 = sub i32 0, %76
	%78 = sub i32 0, %77
	%79 = sub i32 0, %78
	ret i32 %79
}

define void @_INIT_GLOBAL_VARS(){
	store i32 0, i32* @global_char
	store i32 255, i32* @global_char1
	store i32 4, i32* @global_char2
	store i32 36, i32* @global_char3
	store i32 228, i32* @global_char4
	store i32 65, i32* @global_a
	store i32 66, i32* @global_b
	store i32 67, i32* @global_c
	store double 4.0, double* @global_d
	ret void
}