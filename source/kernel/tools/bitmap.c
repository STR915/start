/**
 * λͼ���ݽṹ
 *
 * ���ߣ�����ͭ
 * ��ϵ����: 527676163@qq.com
 */
#include "tools/bitmap.h"
#include "tools/klib.h"

/**
 * @brief ��ȡ����Ҫ���ֽ�����
 */
int bitmap_byte_count (int bit_count) {
    return (bit_count + 8 - 1) / 8;         // ����ȡ��
}

/**
 * @brief λͼ��ʼ��
 */
void bitmap_init (bitmap_t * bitmap, uint8_t * bits, int count, int init_bit) {
    bitmap->bit_count = count;
    bitmap->bits = bits;

    int bytes = bitmap_byte_count(bitmap->bit_count);
    kernel_memset(bitmap->bits, init_bit ? 0xFF: 0, bytes);
}

/**
 * @brief ��������N��λ
 */
void bitmap_set_bit (bitmap_t * bitmap, int index, int count, int bit) {
    for (int i = 0; (i < count) && (index < bitmap->bit_count); i++, index++) {
        // ���Կ��ǽ���һ��������Ż�!!
        if (bit) {
            bitmap->bits[index / 8] |= 1 << (index % 8);
        } else {
            bitmap->bits[index / 8] &= ~(1 << (index % 8));
        }
    }
} 

/**
 * @brief ��ȡָ��λ��״̬
 */
int bitmap_get_bit (bitmap_t * bitmap, int index) {
    //return bitmap->bits[index / 8] & (1 << (index % 8));
    // 2023-3-9 ����Ӧ�÷���0����1
    return (bitmap->bits[index / 8] & (1 << (index % 8))) ? 1 : 0;
}

/**
 * @brief ���ָ��λ�Ƿ���1
 */
int bitmap_is_set (bitmap_t * bitmap, int index) {
    return bitmap_get_bit(bitmap, index) ? 1 : 0;
}

/**
 * @brief ������������ָ������λ��������ʼ����
 */
int bitmap_alloc_nbits (bitmap_t * bitmap, int bit, int count) {
    int search_idx = 0;
    int ok_idx = -1;

    while (search_idx < bitmap->bit_count) {
        // ��λ����һ����ͬ��������
        if (bitmap_get_bit(bitmap, search_idx) != bit) {
            // ��ͬ������Ѱ����ʼ��bit
            search_idx++;
            continue;
        }

        // ��¼��ʼ����
        ok_idx = search_idx;

        // ����������һ����
        int i;
        for (i = 1; (i < count) && (search_idx < bitmap->bit_count); i++) {
            if (bitmap_get_bit(bitmap, search_idx++) != bit) {
                // ����count�����˳������½��������ıȽ�
                ok_idx = -1;
                break;
            }
        }

        // �ҵ������ø�λ��Ȼ���˳�
        if (i >= count) {
            bitmap_set_bit(bitmap, ok_idx, count, ~bit);
            return ok_idx;
        }
    }

    return -1;
}

