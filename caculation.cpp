// 20190914caculation.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc.h"


// 14
int error[100][2] = {};
int line_num = 0;
int error_idx = 0;
struct symbol_list
{
    // Need to detect the length of the name
    char name[10] = {};
    // Type defaults to integer
    int type = 0;
    // Value using double precision for storage
    double value = 0;
    // Next node
    struct symbol_list *next = NULL;
};

struct content_list
{
    char content[100] = {};
    struct content_list *next = NULL;
};

// Read the contents of the file into an array
int read_file(FILE * fp, struct content_list * file_content)
{
    int line_num = 0;
    char line[100] = {};
    int i = 0;
    struct content_list * temp_file_content = file_content;
    struct content_list * temp_file_next = NULL;
    if ( fp == NULL )
    {
        printf( "Could not open file test.c" ) ;
        error[error_idx][0] = 15;
        error[error_idx][1] = line_num;
        error_idx++;
        // 错误15为 不能打开文件
        return 15;
    }
    while(!feof(fp))
    {
		fgets(line, 100, fp);
		if(strlen(line)<100 && strlen(line))
		{
			file_content->next = (struct content_list *) malloc(sizeof(struct content_list));
			file_content = file_content->next;
			strcpy_s(file_content->content, 100, line);
			// printf("%s", line);
			memset(line, 0, sizeof(line));
		}
        else if(strlen(line)>=100)
            // 4错误为 每行不能超过100个字符
        {
            error[error_idx][0] = 4;
            error[error_idx][1] = line_num;
            error_idx++;
            return 4;
        }

		file_content->next = NULL;
    }
    fclose(fp);
    // 由于每一行可能有多句话，需要提前对句子进行拆分，将一个节点拆分成多个节点,并且假如遇到.的时候要释放后续的节点
    // 拆分代码如下(待测试)
    file_content = temp_file_content;
    while(file_content->next)
    {
        // 先将file_content->content的最后一位置为\x00
        file_content->content[99] = '\x00';
        i = 0;
        while((file_content->content[i] != ';' || file_content->content[i] != '.') && file_content->content[i] !='\x0')
        {
            i++;
        }
        // 此时会遇到；了，看后面还有没有字符,假如没有字符就继续循环，跳过后面的部分
		if (file_content->content[i] == ';' && (file_content->content[i + 1] == '\x0'|| file_content->content[i + 1] == '\n'))
			continue;
		// 假如后面没有字符了但是没遇到；则报错
        // 假如以.结尾了而且后面没有字符,将后续的节点释放并将next节点置为null
        else if(file_content->content[i] == '.' && file_content->content[i+1] == '\x0')
        {
            temp_file_content = file_content;
            while(file_content->next)
            {
                temp_file_next = file_content->next;
                file_content->next = file_content->next->next;
                free(temp_file_next);
                temp_file_next = NULL;
            }
            return 0;
        }
        // 假如是;后面还有字符
        else if((file_content->content[i] >= 'a'&&file_content->content[i]<='z')||(file_content->content[i] >='A'&&file_content->content[i]<='Z')||file_content->content[i]==' ')
        {
            temp_file_next = file_content->next;
            file_content->next = (struct content_list *) malloc(sizeof(struct content_list));
            file_content->next->next = temp_file_next;
            temp_file_next = NULL;
            // 然后将后面的内容复制到新节点上
            strcpy_s(file_content->next->content, 100, &file_content->content[i+1]);
        }
        else if (file_content->content[i + 1] == '\x0' || file_content->content[i + 1] == '\n')
        {
            error[error_idx][0] = 3;
            error[error_idx][1] = line_num;
            error_idx++;
            // 3错误为 句子必须以；或.结尾
            return 3;
        }
        // 移动到下一个节点
        file_content = file_content->next;
		line_num++;
    }
	line_num = 0;
    
    return 0;
}

// If no keyword returns 0, if there is a keyword, return the index of the keyword
int parsing_keywords(char *line, int *keyword_index, char *rest_content)
{
    // 先剔除空格
    int i = 0;
    while(line[i]==' ')
        i++;
    // 然后判断从这个字符开始到下个空格之间有没有keyword
    if(line[i] == 'f' && line[i+1] == 'l' && line[i+2] == 'o' && line[i+3] == 'a' && line[i+4] == 't' && line[i+5] == ' ')
    {
        *keyword_index = 1;
        i = i+6;
    }
    else if(line[i] == 'i' && line[i+1] == 'n' && line[i+2] == 't' && line[i+3] == ' ')
    {
        *keyword_index = 2;
        i = i+4;
    }
    else if(line[i] == 'w' && line[i+1] == 'r' && line[i+2] == 'i' && line[i+3] == 't' && line[i+4] == 'e' && line[i+5] == '(')
    {
        *keyword_index = 3;
        i = i+6;
    }
    else
        *keyword_index = 0;
    strcpy_s(rest_content, 100-i, &line[i]);
	return 0;
}

int new_variable(struct symbol_list *symbol_table, char *name, int type)
{
	while (symbol_table->next!=NULL)
	{
		symbol_table = symbol_table->next;
	}
    symbol_table->next = (struct symbol_list *) malloc(sizeof(struct symbol_list));
    strcpy_s(symbol_table->next->name, 10, name);
    symbol_table->next->type = type;
	symbol_table->next->value = 0;
	symbol_table->next->next = NULL;
	return 0;
    // to do
}


int modify_variable(struct symbol_list *symbol_table, char *name, double value, int type)
{
    while(symbol_table->next)
    {
        if(strcmp(name, symbol_table->next->name))
        {
            symbol_table = symbol_table->next;
            continue;
        }
        else
        {
            if(symbol_table->next->type == 1 && type == 2)
            {
                error[error_idx][0] = 6;
                error[error_idx][1] = line_num;
                error_idx++;
                // 错误6为 不能将整数类型的值赋给浮点数
                return 6;
            }
			else
			{
				symbol_table->next->value = value;
			}
            return 0;
        }
    }
    // 错误5为 不能修改未定义变量
    error[error_idx][0] = 5;
    error[error_idx][1] = line_num;
    error_idx++;
	return 5;
}

// Search for a variable, return type and value
int search_variable(struct symbol_list *symbol_table, char *name, int *type, double *value)
{
    while(symbol_table->next)
    {
        if(strcmp(name, symbol_table->next->name))
        {
            symbol_table = symbol_table->next;
            continue;
        }
        else
        {
            *type = symbol_table->next->type;
            *value = symbol_table->next->value;
            return 0;
        }
    }
    error[error_idx][0] = 12;
    error[error_idx][1] = line_num;
    error_idx++;
    // 错误12为 表达式中有未声明的变量
	return 12;
}

int parsing_name_value(char *rest_content, char *name)
{
    // 先剔除空格
    int i = 0;
    int j = 0;
    while(rest_content[i]==' ')
        i++;
    if((rest_content[i]>='a'&&rest_content[i]<='z')||(rest_content[i]>='A'&& rest_content[i]<='Z'))
    {
        name[j] = rest_content[i];
        i++;
        j++;
    }
    while(((rest_content[i]>='a'&&rest_content[i]<='z')||(rest_content[i]>='A'&&rest_content[i] <='Z')||(rest_content[i]>='0'&& rest_content[i] <='9'))&&j<10)
    {
        name[j] = rest_content[i];
        i++;
        j++;
    }
    // 假如读完词之后，读到的第一个不是空格就报错
    if(rest_content[i]!=' '&&rest_content[i]!=';'&&rest_content[i]!='.')
    {
        error[error_idx][0] = 2;
        error[error_idx][1] = line_num;
        error_idx++;
        return 2;
    //2错误为 变量名要以字母开头，并且只能包含字母与数字 
    }
        // 读取完了关键词，之后判断词句后面是不是只有分号/点号与空格
    while(rest_content[i]==' ')
        i++;
    if(rest_content[i]==';'||rest_content[i]=='.')
    {
        name[9] = '\x0';
    	return 0;
    }
    error[error_idx][0] = 1;
    error[error_idx][1] = line_num;
    error_idx++;

    // 1错误为 不能同时初始多个变量
    return 1;
}



int postfix_expression(char *expr, char *postfix_expr)
{
    int len = strlen(expr);
    int i = 0;
    int j = 0;
    char stack[100] = {};
    int stack_idx = -1;
    for(i=0; i<len; i++)
    {
        if(expr[i]>='0'&&expr[i]<='9'||expr[i]=='.')
        {
            postfix_expr[j] = expr[i];
            j++;
        }
        else if(expr[i] == '(')
        {
            if(postfix_expr[j-1] != '#')
            {
                postfix_expr[j] = '#';
                j++;
            }
            while(stack_idx>=0 && stack[stack_idx] != '(')
            {
                postfix_expr[j] = stack[stack_idx];
                stack_idx--;
                j++;
            }
            stack[++stack_idx] = '(';
        }
        else if(expr[i] == ')')
        {
            if(postfix_expr[j-1] != '#')
            {
                postfix_expr[j] = '#';
                j++;
            }
            while(stack_idx>=0 && stack[stack_idx] != '(')
            {
                postfix_expr[j] = stack[stack_idx];
                stack_idx--;
                j++;
            }
            stack_idx--;
        }
        else if(expr[i] == '*' || expr[i] == '/')
        {
            if(postfix_expr[j-1] != '#')
            {
                postfix_expr[j] = '#';
                j++;
            }
            while(stack_idx>=0 && (stack[stack_idx] == '*' || stack[stack_idx] == '/'))
            {
                postfix_expr[j] = stack[stack_idx];
                stack_idx--;
                j++;
            }
            stack[++stack_idx] = expr[i];
        }
        else if(expr[i] == '+' || expr[i] == '-')
        {
            if(postfix_expr[j-1] != '#')
            {
                postfix_expr[j] = '#';
                j++;
            }
            while(stack_idx>=0 &&(stack[stack_idx] == '*' || stack[stack_idx] == '/' || stack[stack_idx] == '+' || stack[stack_idx] == '-'))
            {
                postfix_expr[j] = stack[stack_idx];
                stack_idx--;
                j++;
            }
            stack[++stack_idx] = expr[i];
        }
    }
	while (stack_idx >= 0)
	{
		postfix_expr[j] = stack[stack_idx];
		stack_idx--;
		j++;
	}
	postfix_expr[j] = '\x00';
    return 0;
}

int calc_postfix_expr(char *postfix_expr, double *result)
{
    double num_stack[51] = {};
    int num_idx = -1;
    int i = 0;
    double num = 0;
    char str[20] = {};
    int j = 0;
    while(postfix_expr[i] != '\x00')
    {
		j = 0;
		if (postfix_expr[i] == '#')
			i++;
		else 
		{
			while (postfix_expr[i] != '#' && postfix_expr[i] != '+'&& postfix_expr[i] != '-'&& postfix_expr[i] != '*'&& postfix_expr[i] != '/' && postfix_expr[i] != '\x00')
			{
				str[j] = postfix_expr[i];
				i++;
				j++;
			}
			if (str[0] == '.')
			{
				int k = 0;
				for (k = strlen(str); k > 0; k--)
				{
					str[k] = str[k - 1];
				}
				str[0] = '0';
			}
			if (strlen(str)!=0)
			{
				num = atof(str);
				memset(str, '\x00', 20);
				num_idx++;
				num_stack[num_idx] = num;
			}
			

			while (postfix_expr[i] == '+' || postfix_expr[i] == '-' || postfix_expr[i] == '*' || postfix_expr[i] == '/')
			{
				switch (postfix_expr[i])
				{
				case '+':num = num_stack[num_idx - 1] + num_stack[num_idx]; num_idx--; num_stack[num_idx] = num; break;
				case '-':num = num_stack[num_idx - 1] - num_stack[num_idx]; num_idx--; num_stack[num_idx] = num; break;
				case '*':num = num_stack[num_idx - 1] * num_stack[num_idx]; num_idx--; num_stack[num_idx] = num; break;
				case '/':if (num_stack[num_idx]<0.000001 || num_stack[num_idx]>-0.000001) { num = num_stack[num_idx - 1] / num_stack[num_idx]; num_idx--; num_stack[num_idx] = num; break; }
						 else
							 // 错误14为 0不能作为除数
                         {
                            error[error_idx][0] = 14;
                            error[error_idx][1] = line_num;
                            error_idx++;
        					return 14;
                        }
				}
				i++;
			}
		}
        
    }
	*result = num_stack[0];
    // to do 
	return 0;
}



// 处理空格以及变量
int preprocessing_expression(struct symbol_list *symbol_table, char *raw_expr, char *expr, int *out_type)
{
	int i = 0;
	int j = 0;
	bool pre_num = false;
	bool pre_blank = true;
	bool pre_calc = true;
	bool pre_bracket = false;
	int brackets = 0;
	char name[10] = {};
	int k = 0;
	struct symbol_list * temp_symbol_table = symbol_table;
	int type = 0;
	double value = 0;
	char str[20] = {};
    *out_type = 2;
	bool first_num = false;
	while (raw_expr[i]!=';')
	{
		if (raw_expr[i] == ' ')
		{
			pre_blank = true;
			pre_num = false;
			i++;
		}
		// +)   (+
		else if (raw_expr[i] == '(')
		{
			expr[j] = raw_expr[i];
			pre_bracket = true;
			brackets++;
			i++;
			j++;
		}
		else if (raw_expr[i] == ')' && !pre_calc)
		{
			expr[j] = raw_expr[i];
			brackets--;
			i++;
			j++;
		}
		else if (raw_expr[i] == ')' && pre_calc)
        {
            error[error_idx][0] = 11;
            error[error_idx][1] = line_num;
            error_idx++;
            // 错误11为 右括号前面不能是运算符
            return 11;
        }
			
		// 用于处理第一个数字是负数的情况
		else if (raw_expr[i] == '-' && !first_num)
		{
			expr[j] = '0';
			j++;
			first_num = true;
			pre_num = true;
		}
		// ' #156'
		// ' 1#56'
		// ' 1#.56'
		// ' #.56'
		// ' (#156'
		else if ((raw_expr[i] >= '0' && raw_expr[i] <= '9' || raw_expr[i] == '.') && (pre_calc && pre_blank) || pre_num)
		{
            if(raw_expr[i] == '.')
                *out_type = 1;
			expr[j] = raw_expr[i];
			first_num = true;
			pre_calc = false;
			pre_blank = false;
			pre_num = true;
			pre_bracket = false;
			i++;
			j++;

		}
		// ' 156 157'
		else if ((raw_expr[i] >= '0' && raw_expr[i] <= '9') && !pre_calc && pre_blank)
        {
            error[error_idx][0] = 7;
            error[error_idx][1] = line_num;
            error_idx++;
			// 错误7为 两个数字之间不能没有运算符
			return 7;
        }
		// ' 156 #+ 157'
		// ' 156#+ 157'
		else if (raw_expr[i] == '+' || raw_expr[i] == '-' || raw_expr[i] == '*' || raw_expr[i] == '/' && !pre_calc && !pre_bracket && (pre_blank || pre_num))
		{
			expr[j] = raw_expr[i];
			pre_calc = true;
			pre_blank = false;
			pre_num = false;
			pre_bracket = false;
			i++;
			j++;
		}
		// '++'
		// '+ +'
		else if (raw_expr[i] == '+' || raw_expr[i] == '-' || raw_expr[i] == '*' || raw_expr[i] == '/' && pre_calc)
        {
            error[error_idx][0] = 8;
            error[error_idx][1] = line_num;
            error_idx++;
			// 错误8为 两个运算符之间不能没有数字
			return 8;
        }
		else if (raw_expr[i] == '+' || raw_expr[i] == '-' || raw_expr[i] == '*' || raw_expr[i] == '/' && pre_bracket)
        {
            error[error_idx][0] = 9;
            error[error_idx][1] = line_num;
            error_idx++;
            // 错误9为 运算符之前不能是左括号
            return 9;
        }
			
		// 变量前面必须不能是数字，而且前面出现运算符
		else if ((raw_expr[i] >= 'a' && raw_expr[i] <= 'z') || (raw_expr[i] >= 'A' && raw_expr[i] <= 'Z') && !pre_num && pre_calc)
		{
			name[k] = raw_expr[i];
			i++;
			k++;
			while ((raw_expr[i] >= 'a' && raw_expr[i] <= 'z') || (raw_expr[i] >= 'A' && raw_expr[i] <= 'Z') || (raw_expr[i] >= '0' && (raw_expr[i] <= '9')))
			{
				name[k] = raw_expr[i];
				i++;
				k++;
				if (k == 9)
					break;
			}
			name[k] = '\x00';
			search_variable(temp_symbol_table, name, &type, &value);
			if (type == 1)
			{
				sprintf_s(str, 19, "%.4f", float(value));
				strcpy_s(&expr[j], 19, str);
				j += strlen(str);
				memset(str, 0, sizeof(str));
                *out_type = 1; 
			}
			else if (type == 2)
			{
				sprintf_s(str, 19, "%d", int(value));
				strcpy_s(&expr[j], 19, str);
				j += strlen(str);
				memset(str, 0, sizeof(str));
			}
			first_num = true;
			pre_calc = false;
			pre_blank = false;
			pre_num = true;
			pre_bracket = false;
			k = 0;
		}
		else
        {
            error[error_idx][0] = 13;
            error[error_idx][1] = line_num;
            error_idx++;
			// 错误13为 有非法字符
			return 13;
        }
		
	}
	if (brackets)
    {
        error[error_idx][0] = 10;
        error[error_idx][1] = line_num;
        error_idx++;
        // 错误10为 括号数量不匹配
        return 10;
    }
		
	return 0;
	// to do
}


int parsing_assignment_statement(struct symbol_list *symbol_table, char *rest_content)
{
    int i = 0;
    int j = 0;
    char name[10] = {};
    char expr[100] = {};
    char postfix_expr[100] = {};
    double result = 0;
    int type = 0;
    // 先剔除前面的空格
    while(rest_content[i]==' ')
        i++;
    if((rest_content[i]>='a'&&rest_content[i]<='z')||(rest_content[i]>='A'&& rest_content[i]<='Z'))
    {
        name[j] = rest_content[i];
        i++;
        j++;
    }
    while(((rest_content[i]>='a'&&rest_content[i]<='z')||(rest_content[i]>='A'&&rest_content[i] <='Z')||(rest_content[i]>='0'&& rest_content[i] <='9'))&&j<10)
    {
        name[j] = rest_content[i];
        i++;
        j++;
    }
    // 解析完名称之后继续解析出表达式
    while(rest_content[i]==' ')
        i++;
    if(rest_content[i]=='=')
    {
        i++;
        while(rest_content[i]==' ')
            i++;
        // 在去掉=后面的空格之后就得到了表达式
        // 然后对表达式进行预处理（将表达式中的变量转化为常量）
        preprocessing_expression(symbol_table, &rest_content[i], expr, &type);
        postfix_expression(expr, postfix_expr);
        calc_postfix_expr(postfix_expr, &result);
        modify_variable(symbol_table, name, result, type);

    }
    // 假如没有=符号就直接忽略后面的操作，并返回0
	return 0;
}



// b);
int print_result(struct symbol_list *symbol_table, char *rest_content)
{
	int i = 0;
	char name[10] = {};
	int j = 0;
	int type = 0;
	double value = 0;

	for (; i < 9 && rest_content[i] != ')'; i++)
	{
		name[j] = rest_content[i];
		j++;
	}
	search_variable(symbol_table, name, &type, &value);
	if (type == 1)
		printf("%s的值为%.4f\n", name, value);
	else
		printf("%s的值为%d\n", name,int(value));
	return 0;
    // to do
}

int parsing_contents(struct content_list *file_content, struct symbol_list *symbol_table)
{
    struct content_list *temp_file_content = file_content;
	struct symbol_list *temp_symbol_table = symbol_table;
    while(temp_file_content -> next != NULL)
    {
        // 处理每一行，然后释放
        int index = 0;
        char rest_content[100] = {};
        char name[10] = {};
        parsing_keywords(file_content -> next -> content, &index, rest_content);
        // 假如有关键词1，2则要在符号表中新增节点，假如为3则要打印输出
        // 创建新变量的时候是没有赋值的默认为0
        if(index == 1 || index == 2)
        {
            parsing_name_value(rest_content, name);
            new_variable(temp_symbol_table, name, index);
        }
        else if(index == 3)
        {
            print_result(temp_symbol_table, rest_content);
        }
        else
        {
            // 假如没有关键词又不是赋值语句则直接忽略, 假如是赋值语句则更新符号表
            parsing_assignment_statement(temp_symbol_table, rest_content);
        }
		temp_file_content->next = temp_file_content->next->next;
        line_num++;
    }
	return 0;
}

void error_print()
{
    for(int i=0;i<error_idx;i++)
    {
        switch(error[i][0])
        {
		case 1:printf("第%d句话中有错误>>不能同时初始多个变量\n", error[i][1]); break;
		case 2:printf("第%d句话中有错误>>变量名要以字母开头，并且只能包含字母与数字\n", error[i][1]); break;
            case 3:printf("第%d句话中有错误>>句子必须以；或.结尾\n", error[i][1]); break;
            case 4:printf("第%d句话中有错误>>每行不能超过100个字符\n", error[i][1]); break;
            case 5:printf("第%d句话中有错误>>不能修改未定义变量\n", error[i][1]); break;
            case 6:printf("第%d句话中有错误>>不能将整数类型的值赋给浮点数\n", error[i][1]); break;
            case 7:printf("第%d句话中有错误>>两个数字之间不能没有运算符\n", error[i][1]); break;
            case 8:printf("第%d句话中有错误>>两个运算符之间不能没有数字\n", error[i][1]); break;
            case 9:printf("第%d句话中有错误>>运算符之前不能是左括号\n", error[i][1]); break;
            case 10:printf("第%d句话中有错误>>括号数量不匹配\n", error[i][1]); break;
            case 11:printf("第%d句话中有错误>>右括号前面不能是运算符\n", error[i][1]); break;
            case 12:printf("第%d句话中有错误>>表达式中有未声明的变量\n", error[i][1]); break;
            case 13:printf("第%d句话中有错误>>有非法字符\n", error[i][1]); break;
            case 14:printf("第%d句话中有错误>>0不能作为除数\n", error[i][1]); break;
            case 15:printf("第%d句话中有错误>>打开文件失败\n", error[i][1]); break;
        }
    }
}


int main(int argc, char *argv[])
{
    FILE * fp = NULL;
    if(argc == 1)
    {
        fopen_s(&fp, "source.txt", "r" );
    }
    else
    {
        fopen_s(&fp, argv[1], "r" );
    }
    // Keyword table
    char keyword[3][10] = {"float", "int", "write"};
    // Symbol table(symbol list)
    struct symbol_list *symbol_table_head = (struct symbol_list *) malloc(sizeof(struct symbol_list));
	symbol_table_head->next = NULL;
    // File_content is a two-dimensional array for storing the contents of a file(content_list)
    struct content_list *file_content_head = (struct content_list *) malloc(sizeof(struct content_list));
	file_content_head->next = NULL;

    read_file(fp, file_content_head);
    parsing_contents(file_content_head, symbol_table_head);
	error_print();
	system("PAUSE");
    return 0;
}



// this main function for testing
// int main()
//{
    // test for read_file()
    // FILE *fp = NULL;
    // struct content_list *file_content_head = (struct content_list *) malloc(sizeof(struct content_list));
    // read_file(fp, file_content_head);

    // test for parsing_keywords()
    // it should return 2 when using line1
    // char line1[100] = "int a;";
    // it should return 0 when using line2
    // char line2[100] = "inta sadfa;";
    // int index = 0;
    // char keyword[3][10] = {"float", "int", "write"};
    // char rest_content[100] = {};
    // parsing_keywords(line2, &index, rest_content);

    // test for parsing_name_value
    // the length of variable should less than 10, otherwise the rest part will be thrown
    // char name[10] = {};
    // char rest_content[100] = "asldfhlnvzlcn;";
    // parsing_name_value(rest_content, name);

    // test for new_variable
 //    struct symbol_list *symbol_table_head = (struct symbol_list *) malloc(sizeof(struct symbol_list));
 //    char name[10] = "asdfvz";
 //    int type = 2;
 //    new_variable(symbol_table_head, name, type);
	// modify_variable(symbol_table_head, name, 55.6, 1);

    // test for parsing_assignment_statement
    // char rest_content[100] = "asdfvz = 15";
    // char rest_content[100] = "a = 15";
    // char rest_content[100] = "a 15";
    // parsing_assignment_statement(symbol_table_head, rest_content);

    // test for preprocessing_expression
    // char rest_content[50] = "  asdfvz + 6;";
    // char expr[50] = {};
    // preprocessing_expression(symbol_table_head, rest_content, expr);

    // test for postfix_expression
//     char expr[100] = "(50+6.8000)*32-.5";
//     char postfix_expr[100] = {};
//     double result = 0;
// 	float fresult = 0;
//     postfix_expression(expr, postfix_expr);
// 	calc_postfix_expr(postfix_expr, &result);
// 	fresult = float(result);
// 	system("PAUSE");
//     return 0;
// }