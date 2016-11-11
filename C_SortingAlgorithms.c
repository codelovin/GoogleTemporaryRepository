
//
//  problem#6779.c
//  mipt-programming
//
// В этом задании Вам необходимо было реализовать сортировки: пузырьковую, быструю и HeapSort, сортировки слиянием и Шелла.
//
// Сравните реализованные сортировки на разных наборах данных. Найдите плохой случай для быстрой. Сравните со стандартной qsort и своей оптимизированной.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "performance.h"

#pragma mark - Общие структуры и функции

// Для удобства введем отсутствующий тип bool
typedef int bool;
#define true 1
#define false 0

// Вспомогательная структура для удобного возврата индексов
typedef struct {
    size_t first;
    size_t second;
} IndexPair;

//Структура, представляющая двоичное дерево
typedef struct {
    void* nodes;
    int size;
} heap_t;

void printHeap(heap_t* heap);

//Пользовательская функция сравнения (может быть изменена)
int cmp(const void *a, const void*b) {
    int a_int = *(int*)a;
    int b_int = *(int*)b;
    if (a_int < b_int) {
        return -1;
    } else if (a_int > b_int) {
        return 2;
    }
    return 0;
}

//Вспомогательная функция замены двух элементов
void swap(void *a, void *b, size_t size) {
    void* tmp = malloc(size);
    memcpy(tmp, b, size);
    memcpy(b, a, size);
    memcpy(a, tmp, size);
    free(tmp);
}

#pragma mark - Функции генерации, рандомизации и вывода массивов

int* generateRandomArray(int size) {
    int* array = (int*) malloc(size*sizeof(int));
    for (int i = 0; i < size; i++) {
        array[i] = rand()%1000;
    }
    return array;
}

int* generateConstantArray(int size) {
    int* array = (int*) malloc(size*sizeof(int));
    for (int i = 0; i < size; i++) {
        array[i] = 123456;
    }
    return array;
}

int* generateAscendingArray(int size) {
    int* array = (int*) malloc(size*sizeof(int));
    for (int i = 0; i < size; i++) {
        array[i] = i;
    }
    return array;
}

int* generateDescendingArray(int size) {
    int* array = (int*) malloc(size*sizeof(int));
    for (int i = 0; i < size; i++) {
        array[i] = size - i - 1;
    }
    return array;
}

void randomShuffle(int *array, size_t n)
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void printArray(int* arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

#pragma mark - Проверка на отсортированность

// Функция которая за O(N) проходится по массиву и считает отсортированный по возрастанию и по убыванию подмассив максимальной длины
IndexPair getIndexesOfOptimalSortedSubarray(void* array, int N, size_t size, int(*compare)(const void*, const void*)) {
    size_t maximalLength = 1;
    IndexPair maxIndexes = {0, 0};
    size_t leftIndex = 0;
    for (int i = 1; i < N; i++) {
        if (compare(array+i*size, array+(i-1)*size)!=-1) continue;
        if (i - leftIndex > maximalLength) {
            maximalLength = i - leftIndex;
            maxIndexes.first = leftIndex;
            maxIndexes.second = i - 1;
            leftIndex = i;
        }
    }
    if (N - leftIndex > maximalLength) {
        maximalLength = N - leftIndex;
        maxIndexes.first = leftIndex;
        maxIndexes.second = N - 1;
    }
    int maximalLength_desc = 1;
    int leftIndex_desc = 0;
    IndexPair maxIndexes_desc = {0, 0};
    for (int i = 1; i < N; i++) {
        if (compare(array+i*size, array+(i-1)*size)!=2) continue;
        if (i - leftIndex > maximalLength_desc) {
            maximalLength_desc = i - leftIndex_desc;
            maxIndexes_desc.first = leftIndex_desc;
            maxIndexes_desc.second = i - 1;
            leftIndex_desc = i;
        }
    }
    if (N - leftIndex_desc > maximalLength_desc) {
        maximalLength_desc = N - leftIndex_desc;
        maxIndexes_desc.first = leftIndex_desc;
        maxIndexes_desc.second = N - 1;
    }
    if (maximalLength_desc > maximalLength) {
        return maxIndexes_desc;
    }
    return maxIndexes;
}

int checkIfSorted(int* array, int n, size_t size, int(*compare)(const void*, const void*)) {
    IndexPair pair = getIndexesOfOptimalSortedSubarray(array, n, size, compare);
    if (pair.first == 0 && pair.second == n - 1) {
        return 1;
    }
    return 0;
}

#pragma mark - Функция тестирования сортировки на производительность
double measureSort(void(*sort_)(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size), int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    startPerformanceCounter();
    sort_(compare, array, l, r, size);
    return getPerformanceCounter();
}

#pragma mark - Начальный qsort

//Функция partition (метод Хоара)
int partition_sqsort(void* array, int l, int r, size_t size, int(*compare)(const void*, const void*)) {
    void *pivot = malloc(size);
    memcpy(pivot, array+l*size, size);
    int i = l-1;
    int j = r+1;
    while(1)
    {
        do ++i; while(i <= r && compare(array+i*size, pivot)==-1);
        do --j; while(compare(pivot, array+j*size)==-1);
        if (i >= j) break;
        swap(array+i*size, array+j*size, size);
    }
    free(pivot);
    return j;
}

//Главная функция сортировки
void quickSort_weak(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    if (l >= r)
        return;
    int i = partition_sqsort(array, l, r, sizeof(int), compare);
    quickSort_weak(compare, array, l, i, size);
    quickSort_weak(compare, array, i+1, r, size);
}

#pragma mark - Оптимизированный qsort

//Функция, возвращающая медианное значение трех элементов
void* getMedian(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    void*tmp1 = malloc(size);
    void*tmp2 = malloc(size);
    void*tmp3 = malloc(size);
    memcpy(tmp1, array+(l+rand()%(r-l))*size, size);
    memcpy(tmp2, array+(l+rand()%(r-l))*size, size);
    memcpy(tmp3, array+(l+rand()%(r-l))*size, size);
    if (compare(tmp1, tmp2)==2) {
        swap(tmp1, tmp2, size);
    }
    if (compare(tmp2, tmp3)==2) {
        swap(tmp2, tmp3, size);
    }
    free(tmp1);
    free(tmp3);
    return tmp2;
}

//Функция partition (метод Хоара)
//Одна из оптимизаций - брать медианный элемент из трех
int partition_optqsort(void* array, int l, int r, size_t size, int(*compare)(const void*, const void*)) {
    void *pivot = getMedian(compare, array, l, r, size);
    int i = l-1;
    int j = r+1;
    while(1)
    {
        do ++i; while(i <= r && compare(array+i*size, pivot)==-1);
        do --j; while(compare(pivot, array+j*size)==-1);
        if (i >= j) break;
        swap(array+i*size, array+j*size, size);
    }
    free(pivot);
    return j;
}

//Функция сортировки без вставок
//Вторая оптимизация - не сортировать массив методом qsort, если в нем менее 40 элементов
void quickSort_optimised_noIns(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    if (r-l<=40)
        return;
    int i = partition_optqsort(array, l, r, sizeof(int), compare);
    quickSort_optimised_noIns(compare, array, l, i, size);
    quickSort_optimised_noIns(compare, array, i+1, r, size);
}

//Функция сортировки вставкой
void insertionSort(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    for(size_t i = 1; i <= r; ++i) {
        void* tmp = malloc(size);
        memcpy(tmp, array+i*size, size);
        size_t j = i;
        while(j > 0 && compare(tmp, array+(j - 1)*size)==-1) {
            memcpy(array+j*size, array+(j-1)*size, size);
            --j;
        }
        memcpy(array+j*size, tmp, size);
        free(tmp);
    }
}

//Главная функция оптимизированной быстрой сортировки
void quickSort_optimised(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    quickSort_optimised_noIns(compare, array, l, r, size);
    insertionSort(compare, array, l, r, size);
}

#pragma mark - Стандартный qsort
void qsort_standard(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    qsort(array, r-l+1, size, compare);
}

#pragma mark - Сортировка Шелла

//Главная функция сортировки Шелла
void shellSort(int (*compare)(const void*, const void*), void *array, int l, int r, size_t size) {
    int n = r - l + 1;
    for(int k = n/2; k > 0; k /=2) {
        for(int i = k; i < n; i++)
        {
            void* t = malloc(size);
            memcpy(t, array+i*size, size);
            int j = 0;
            for(j = i; j>=k; j-=k)
            {
                if(compare(t, array+(j-k)*size)==-1)
                    memcpy(array+j*size, array+(j-k)*size, size);
                else
                    break;
            }
            memcpy(array+j*size, t, size);
            free(t);
        }
    }
}

#pragma mark - Пузырьковая сортировка

void bubbleSort(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    for (int i = 0; i < r - l; i++) {
        for (int j = 0; j < r - l - i; j++) {
            if (compare(array+j*size, array+(j+1)*size)==2) {
                swap(array+j*size, array+(j+1)*size, size);
            }
        }
    }
}

#pragma mark - Сортировка слиянием

//Функция, "сливающая" два отсортированных массива вместе
void mergeArrays(int (*compare)(const void*, const void*), void* array, int low, int med, int high, size_t size) {
    void *total = malloc((high-low+1)*size);
    int index_l = low;
    int index_r = med+1;
    int i = 0;
    while (index_l <= med && index_r <= high) {
        if (compare(array+index_l*size, array+index_r*size)==-1 || compare(array+index_l*size, array+index_r*size)==0) {
            memcpy(total+i*size, array+index_l*size, size);
            i++;
            index_l++;
        } else {
            memcpy(total+i*size, array+index_r*size, size);
            i++;
            index_r++;
        }
    }
    if (index_l>med && index_r > high) {
        ;
    } else if (index_l>med) {
        int q = high - index_r + 1;
        for (int j = 0; j < q; j++) {
            memcpy(total+i*size, array+index_r*size, size);
            ++i;
            ++index_r;
        }
    } else {
        int q = med - index_l + 1;
        for (int j = 0; j < q; j++) {
            memcpy(total+i*size, array+index_l*size, size);
            i++;
            index_l++;
        }
    }
    for (int j = 0;j<high-low+1; j++) {
        memcpy(array+(low+j)*size, total+j*size, size);
    }
    free(total);
}

//Главная функция mergesort
void mergeSort(int (*compare)(const void*, const void*), void* array, int low, int high, size_t size) {
    if (low >= high) {
        return;
    }
    int med = (low+high)/2;
    mergeSort(*compare, array, low, med, size);
    mergeSort(*compare, array, med+1, high, size);
    mergeArrays(*compare, array, low, med, high, size);
}

#pragma mark - Heap Sort (Пирамидальная сортировка)

//Функция, восстанавливающая свойство двоичного дерева с корнем в i-м индексе
void maxHeapify(heap_t *heap, int i, size_t size) {
    int l = 2*i+1;
    int r = 2*i+2;
    int largest = i;
    if (l <= heap->size-1 && cmp(heap->nodes+l*size, heap->nodes+largest*size)==2) {
        largest = l;
    }
    if (r <= heap->size-1 && cmp(heap->nodes+r*size, heap->nodes+largest*size)==2) {
        largest = r;
    }
    if (largest != i) {
        swap((heap->nodes)+i*size, (heap->nodes)+largest*size, size);
        maxHeapify(heap, largest, size);
    }
}

//Функция, строющая корректное двоичное дерево по заданному дереву
void buildHeap(heap_t* heap, size_t size) {
    for (int i = (heap->size) - 1; i >= 0; i--) {
        maxHeapify(heap, i, size);
    }
}

//Функция, добавляющая новый элемент в уже упорядоченное дерево
void addNode(heap_t* heap, void* node, size_t size) {
    realloc(heap->nodes, size*(heap->size++));
    memcpy(heap->nodes+(heap->size-1)*size, node, size);
    int i = heap->size-1;
    int parent = (i-1)/2;
    while (i > 0 && cmp(heap->nodes+parent*size, heap->nodes+i*size)==-1) {
        swap(heap->nodes+parent*size, heap->nodes+i*size, size);
        i = parent;
        parent = (i-1)/2;
    }
}

//Функция, удаляющая максимальный (корневой) элемент
void removeRoot(heap_t* heap, size_t size) {
    memcpy(heap->nodes, heap->nodes+(heap->size-1)*size, size);
    heap->nodes = realloc(heap->nodes, size*(heap->size-1));
    heap->size--;
    maxHeapify(heap, 0, size);
}

//Вспомогательная функция, выводящая двоичное дерево в стандартный поток вывода (для тестирования)
void printHeap(heap_t *heap) {
    for (int i = 0; i < heap->size;i++) {
        printf("%d ", *(int*)(heap->nodes+i*sizeof(int)));
    }
    printf("\n");
}

//Функция пирамидальной сортировки
void heapSort_h(void* array, heap_t* heap, size_t size) {
    buildHeap(heap, size);
    int s = heap->size;
    for (int i = 0; i < s; i++) {
        memcpy(array+i*size, heap->nodes, size);
        removeRoot(heap, size);
    }
}

//Главная функция пирамидальной сортировки
void heapSort(int(*compare)(const void*, const void*), void* array, int l, int r, size_t size) {
    heap_t* heap = malloc(sizeof(heap_t));
    heap->nodes = malloc(size*(r-l+1));
    memcpy(heap->nodes, array, size*(r-l+1));
    heap->size = r - l + 1;
    heapSort_h(array, heap, size);
}

#pragma mark - Main

int main() {
    int size = 10000;
    printf("===Random===\n");
    int* array = generateRandomArray(size);
    double result = measureSort(bubbleSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Bubble Sort:            %lf\n", result);
    }
    randomShuffle(array, size);
    result = measureSort(mergeSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Merge Sort:             %lf\n", result);
    }
    randomShuffle(array, size);
    result = measureSort(quickSort_weak, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Weak):      %lf\n", result);
    }
    randomShuffle(array, size);
    result = measureSort(quickSort_optimised, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Optimised): %lf\n", result);
    }
    randomShuffle(array, size);
    result = measureSort(qsort_standard, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Library):   %lf\n", result);
    }
    randomShuffle(array, size);
    result = measureSort(heapSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Heap Sort:              %lf\n", result);
    }
    randomShuffle(array, size);
    result = measureSort(shellSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Shell Sort:             %lf\n", result);
    }
    printf("\n===Sorted===\n");
    result = measureSort(bubbleSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Bubble Sort:            %lf\n", result);
    }
    result = measureSort(mergeSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Merge Sort:             %lf\n", result);
    }
    result = measureSort(quickSort_weak, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Weak):      %lf\n", result);
    }
    result = measureSort(quickSort_optimised, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Optimised): %lf\n", result);
    }
    result = measureSort(qsort_standard, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Library):   %lf\n", result);
    }
    result = measureSort(heapSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Heap Sort:              %lf\n", result);
    }
    result = measureSort(shellSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Shell Sort:             %lf\n", result);
    }
    printf("\n===Constant===\n");
    free(array);
    array = generateConstantArray(size);
    result = measureSort(bubbleSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Bubble Sort:            %lf\n", result);
    }
    result = measureSort(mergeSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Merge Sort:             %lf\n", result);
    }
    result = measureSort(quickSort_weak, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Weak):      %lf\n", result);
    }
    result = measureSort(quickSort_optimised, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Optimised): %lf\n", result);
    }
    result = measureSort(qsort_standard, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Quick Sort (Library):   %lf\n", result);
    }
    result = measureSort(heapSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Heap Sort:              %lf\n", result);
    }
    result = measureSort(shellSort, cmp, array, 0, size-1, sizeof(int));
    if (checkIfSorted(array, size, sizeof(int), cmp)) {
        printf("Shell Sort:             %lf\n", result);
    }
    printf("\n");
    free(array);
    return 0;
}
