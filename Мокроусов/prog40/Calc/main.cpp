
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <math.h> 
#include <ctype.h> 
#include "processor.h" 

int main() {
	TEvaluator evaluator;
	//������� ������� ����, ��� ������ �����������
	evaluator.input = fopen("in.txt", "r");
	if (!evaluator.input)
	{
		printf("ERROR: No input file found!\n");
		return 0;
	}
	//������� �������� ����, ��� ������ �����������
	evaluator.output = fopen("out.txt", "w");
	if (!evaluator.output)
	{
		fclose(evaluator.input);
		printf("ERROR: No output file found!\n");
		return 0;
	}	
	//������� ������� ������
	TEvaluator_GetInput(&evaluator);
	//��������� ����, ��� �������� ������ ���������
	if (!TEvaluator_Check(&evaluator)) {
		fprintf(evaluator.output, "syntax error");
		free(evaluator.string);
		return 0;
	}
	fseek(evaluator.input, 0, SEEK_SET);
	TEvaluator_GetNextSymbol(&evaluator);
	while (evaluator.operation) {
		double exp = TEvaluator_ProcessSumSub(&evaluator);
		//���� ���������� ������� �� ����, ��������
		if (isinf(exp) || isnan(exp)) {
			fprintf(evaluator.output, "division by zero");
			fclose(evaluator.output);
			fclose(evaluator.input);
			return 0;
		}

		int exp_int = (int)exp;
		fprintf(evaluator.output, "%d", exp_int);
		//������� �����
	}
	fclose(evaluator.input);
	fclose(evaluator.output);
	return 0;
}