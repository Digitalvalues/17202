#include <fstream>
#pragma warning (disable : 4996)

using namespace std;

void heap_sort(int *arr, int size);
void heapify(int *arr, int size, int i);
void swap(int *lhs, int *rhs);

const int BUF_SIZE = 100000;

int main() {
	FILE *in = fopen("in.txt", "r");
	FILE *out = fopen("out.txt", "w");
	
	int *arr;
	int n;

	if (in == NULL) {
		fprintf(out, "Couldn't open file\n");
		goto END;
	}

	fscanf(in, "%d", &n);
	if (n < 0) {
		fprintf(out, "n is negative!\n");
		goto END;
	}

	arr = new int[n];
	for (int i = 0; i < n; ++i) { fscanf(in, "%d", arr + i); }

	heap_sort(arr, n);

	setvbuf(out, NULL, _IOFBF, BUF_SIZE);
	for (int i = 0; i < n; ++i) {
		fprintf(out, "%d ", arr[i]);

		if (i % 1024 == 0) {
			fflush(out);
		}
	}

	delete[] arr;

END:
	fclose(in);
	fclose(out);
	return 0;
}

void heap_sort(int *arr, int size) {

	int tempBound;

	for (int i = size - 1; i >= 0; --i) {
		heapify(arr, size, i);
	}

	while (size > 0) {
		heapify(arr, size, 0);

		swap(&arr[--size], &arr[0]);
	}
}

void heapify(int *arr, int size, int i) {
	int left;
	int right;
	int largest = i;
	
	while (true) {
		left = (i << 1) + 1;
		right = left + 1;

		if ((left < size) && (arr[left] > arr[largest])) { largest = left; }
		if ((right < size) && (arr[right] > arr[largest])) { largest = right; }

		if (largest == i) { break; }

		swap(&arr[i], &arr[largest]);
		i = largest;
	}
}

void swap(int *lhs, int *rhs) {
	if (lhs == rhs) { return; }

	*lhs ^= *rhs;
	*rhs ^= *lhs;
	*lhs ^= *rhs;
}