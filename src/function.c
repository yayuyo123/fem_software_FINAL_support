#include <stdio.h>
#include <math.h>
#include "function.h"

/**
 * 配列の要素を順に加算し、累積和が目標値に一致する最初の要素番号を返す関数。
 * 浮動小数点数の計算誤差を考慮し、許容誤差で目標値との一致を確認する。
 *
 * @param arr    double型の配列（加算対象の数値が格納されている）
 * @param size   配列の要素数
 * @param target 累積和が一致すべき目標値
 * @return       目標値と一致した場合はその要素番号（インデックス）、一致しない場合は -1 を返す
 *
 * 使用例:
 * double array[] = {1.1, 2.2, 3.3};
 * int index = findMatchingIndexDouble(array, 3, 6.6);
 * // 結果: index は 2 (0, 1, 2 の累積和が 6.6 に一致)
 */
int find_matching_index_double(double arr[], int size, double target) {
    double sum = 0.0;
    const double EPSILON = 1e-9;  // 許容誤差をdouble型に合わせて調整

    for (int i = 0; i < size; i++) {
        sum += arr[i];
        if (fabs(sum - target) < EPSILON) {  // 許容誤差で一致を確認
            return i;  // 目標値に一致した要素番号を返す
        }
    }
    
    return -1;  // 一致しない場合、エラーコードとして-1を返す
}


/**
 * 配列の要素を指定されたインデックスまで累積加算し、その累積和を返す関数。
 *
 * @param arr    double型の配列（累積加算対象の数値が格納されている）
 * @param size   配列の要素数
 * @param index  累積加算を行う最後の要素番号（インデックス）
 * @return       累積和を返す（インデックスが無効な場合は -1.0 を返す）
 *
 * 使用例:
 * double array[] = {1.1, 2.2, 3.3};
 * double sum = calculateSumToIndex(array, 3, 2);
 * // 結果: sum は 6.6 (0, 1, 2 の累積和)
 */
double calculate_sum_to_index(double arr[], int size, int index) {
    if (index < 0 || index >= size) {
        return -1.0; // インデックスが無効な場合のエラーコード
    }

    double sum = 0.0;

    for (int i = 0; i <= index; i++) { // 指定されたインデックスまでの累積和を計算
        sum += arr[i];
    }

    return sum; // 累積和を返す
}


/**
 * @brief double型の配列から指定された値に最も近いインデックスを探す関数。
 * 
 * @param arr 検索対象の配列 (double型)
 * @param size 配列の要素数
 * @param target 探索する値 (double型)
 * @return int 見つかった要素のインデックス (見つからない場合は -1)
 */
int find_index_double(double* arr, int size, double target)
{
    const double EPSILON = 1e-9;  // 許容誤差をdouble型に合わせて調整
    // 配列を線形探索
    for (int i = 0; i < size; i++)
    {
        if (fabs(arr[i] - target) <= EPSILON)
        {  // 許容誤差内で一致とみなす
            return i;  // 一致した場合のインデックスを返す
        }
    }
    return -1;  // 見つからない場合
}

/**
 * @brief 指定された配列内で、start から始まる連続した差が等しい要素の数をカウントします。
 * 
 * @param start 配列の開始インデックス（カウントの開始位置）
 * @param end 配列の終了インデックス（範囲の終端、比較対象外）
 * @param array 値を保持する配列（double型）
 * @param size 配列の要素数（size_t型）
 * @return int 連続した差が等しい要素の数。エラーの場合は -1 を返す。
 */
int count_consecutive(int start, int end, const double array[], int size)
{
    // 入力の範囲が不正な場合のエラーチェック
    if (start < 0 || end > size || start >= end) 
    {
        printf("[ERROR] count_consecutive: Invalid range (start=%d, end=%d, size=%d).\n", start, end, size);
        return -1; // エラー値を返す
    }

    // 最初の差を計算
    double prev_diff = array[start + 1] - array[start];
    
    // 最初の要素は常にカウントされる
    int count = 1; 

    // 配列内の連続する要素を比較
    for (int i = start + 1; i < end; i++) 
    {
        // i と i+1 の差を計算
        double diff = array[i + 1] - array[i];

        // 差が0.001未満であれば等しいとみなす
        if (fabs(diff - prev_diff) < 0.001) 
        {
            count++; // カウントを増加
            prev_diff = diff; // 差を更新
        } 
        else 
        {
            // 差が異なれば連続性が途切れたとみなして終了
            break; 
        }
    }

    return count; // 連続した差が等しい要素のカウントを返す
}

/**
 * @brief 配列の要素を反転する関数
 * 
 * この関数は、指定された配列内の要素を反転します。
 * 例えば、入力配列が {1, 2, 3, 4} の場合、出力は {4, 3, 2, 1} となります。
 *
 * @param arr[] 配列のポインタ（反転対象）
 * @param size 配列のサイズ（要素数）
 */
void reverse_array(int arr[], int size) {
    // 配列の先頭から中央に向かうループ
    for (int i = 0; i < size / 2; i++) {
        // 現在の要素 arr[i] と対応する末尾側の要素 arr[size - 1 - i] を交換する
        int temp = arr[i];                    // 現在の要素を一時保存
        arr[i] = arr[size - 1 - i];           // 末尾側の要素を現在の位置にコピー
        arr[size - 1 - i] = temp;             // 一時保存しておいた値を末尾側の位置にコピー
    }
}