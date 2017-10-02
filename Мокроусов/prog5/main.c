#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include <windows.h>

void bubble_sort(tree_node *arr, int len)
{
    int i, j;
    for (i=0;i<len;i++)
        for (j=i+1;j<len;j++)
            if (arr[i].node_weight>arr[j].node_weight)
            {
                tree_node item = arr[i];
                arr[i] = arr[j];
                arr[j] = item;
            }
}

long count_frequencies(FILE *in, tree_node *freq_arr)
{
    int i = 0;
    for (i=0;i<256;i++)
    {
        freq_arr[i].node_char = (unsigned char) i;
        freq_arr[i].childs[1] = freq_arr[i].childs[0] = -1;
        freq_arr[i].node_weight = 0;
    }
    int temp;
    long result = 0;
    while ((temp = fgetc(in))!=EOF)
    {
        freq_arr[(unsigned char) temp].node_weight++;
        result++;
    }
    return result;
}

int genHaffmanTree(queue *q1, queue *q2, tree_node *haffman_tree)
{
    if (q1->left+1==q1->right)
    {
        haffman_tree[q2->right].childs[0] = q1->left;
        haffman_tree[q2->right].childs[1] = -1;
        q2->right++;
        return q2->right-1;
    }
    while ((q1->right-q1->left)+(q2->right-q2->left)>=2)
    {
        int node1 = -1;
        int node2 = -1;
        if (q1->left<q1->right)
            node1 = q1->left++;
        if (q2->left<q2->right)
            node2 = q2->left++;
        if ((node1==-1) || ((q2->left<q2->right) && (haffman_tree[q1->left-1].node_weight>haffman_tree[q2->left].node_weight)))
        {
            if (node1!=-1) q1->left--;
            node1 = q2->left++;
        }
        if ((node2==-1) || ((q1->left<q1->right) && (haffman_tree[q2->left-1].node_weight>haffman_tree[q1->left].node_weight)))
        {
            if (node2!=-1) q2->left--;
            node2 = q1->left++;
        }

        haffman_tree[q2->right].node_weight = haffman_tree[node1].node_weight + haffman_tree[node2].node_weight;
         haffman_tree[q2->right].childs[0] = node1;
        haffman_tree[q2->right].childs[1] = node2;
        q2->right++;
    }
    return q2->left;
}

void go_tree(tree_node *haffman_tree, int node_id, char *temp_buffer, unsigned char seq_len, bitseq *bit_sequences)
{
    int i;
    if ((haffman_tree[node_id].childs[0]==-1) && (haffman_tree[node_id].childs[1]==-1))
    {
        unsigned char c = haffman_tree[node_id].node_char;
        bit_sequences[c].count_digits = seq_len;
        for (i=0;i<seq_len;i++)
            bit_sequences[c].data[i] = temp_buffer[i];
        return;
    }
    for (i=0;i<2;i++)
        if (haffman_tree[node_id].childs[i]!=-1)
        {
            temp_buffer[seq_len] = i;
            go_tree(haffman_tree, haffman_tree[node_id].childs[i], temp_buffer, seq_len+1, bit_sequences);
        }
}

void write_map(FILE *out, tree_node *haffman_tree, int startMap, int lastNode)
{
    fwrite(&startMap, sizeof(int), 1, out);
    fwrite(&lastNode, sizeof(int), 1, out);
    int i;
    for (i=startMap;i<=lastNode;i++)
    {
        if (i<256)
        {
            fwrite(&haffman_tree[i].node_char, 1, 1, out);
        }
        else
        {
            fwrite(&(haffman_tree[i].childs[0]), sizeof(short), 1, out);
            fwrite(&(haffman_tree[i].childs[1]), sizeof(short), 1, out);
        }
    }
}

void encode_data(FILE *in, FILE *out, bitseq *bit_sequences, long count_chars)
{
    fwrite(&count_chars, sizeof(long), 1, out);
    unsigned char temp = 0;
    unsigned char count_bits = 0;
    int read_byte, i, counter = 0;
    while ((read_byte=fgetc(in))!=EOF)
    {
        counter++;
        unsigned char temp_char = (unsigned char) read_byte;
        for (i=0;i<bit_sequences[temp_char].count_digits;i++)
        {
            temp <<= 1;
            temp |= bit_sequences[temp_char].data[i];
            count_bits++;
            if (count_bits==8)
            {
                count_bits = 0;
                fwrite(&temp, 1, 1, out);
            }
        }
    };
    if (count_bits!=0)
    {
        temp <<= (8-count_bits);
        fwrite(&temp, 1, 1, out);
    };
}
void compress(FILE *in, FILE *out)
{
    long old_ptr = ftell(in);
    tree_node haffman_tree[1024];
    long count_chars = count_frequencies(in, haffman_tree);
    int start_map = 0;
    bubble_sort(haffman_tree, 256);
    while (!haffman_tree[start_map].node_weight) start_map++;
    queue q1, q2;
    q1.left = start_map;
    q1.right = q2.left = q2.right = 256;
    int lastNode = genHaffmanTree(&q1, &q2, haffman_tree);
    bitseq bit_sequences[256];
    char temp_sequence[256];
    go_tree(haffman_tree, lastNode, temp_sequence, 0, bit_sequences);
    fseek(in, old_ptr, SEEK_SET);
    write_map(out, haffman_tree, start_map, lastNode);
    fputc(0xDE, out);
    fputc(0xAD, out);
    fputc(0xBE, out);
    fputc(0xEF, out);
    encode_data(in, out, bit_sequences, count_chars);
}
int read_haffman_tree(FILE *in, tree_node *haffman_tree)
{
    int startMap, lastNode;
    fread(&startMap, sizeof(int), 1, in);
    fread(&lastNode, sizeof(int), 1, in);
    int i;
    for (i=startMap;i<=lastNode;i++)
    {
        if (i<256)
        {
            fread(&haffman_tree[i].node_char, 1, 1, in);
            haffman_tree[i].childs[0] = -1;
            haffman_tree[i].childs[1] = -1;
        }
        else
        {
            fread(&(haffman_tree[i].childs[0]), sizeof(short), 1, in);
            fread(&(haffman_tree[i].childs[1]), sizeof(short), 1, in);
        }
    }
    return lastNode;
}
void decompress_chars(FILE *in, FILE *out, tree_node *haffman_tree, int last_node)
{
    int count_chars;
    fread(&count_chars, sizeof(long), 1, in);
    int read_byte, i, counter = 0;
    int cur_tree_node = last_node;
    while (((read_byte=fgetc(in))!=EOF) && (count_chars))
    {
        counter++;
        unsigned char ch = (unsigned char) read_byte;
        for (i=0;i<8;i++)
        {
            cur_tree_node = haffman_tree[cur_tree_node].childs[(ch >> (7-i)) & 1];
            if ((haffman_tree[cur_tree_node].childs[0]==-1) && (haffman_tree[cur_tree_node].childs[1]==-1))
            {
                fputc(haffman_tree[cur_tree_node].node_char, out);
                cur_tree_node = last_node;
                count_chars--;
                if (!count_chars) break;
            }
        }
    };
}
void decompress(FILE *in, FILE *out)
{
    tree_node haffman_tree[1024];
    int last_node = read_haffman_tree(in, haffman_tree);
    int i;
    volatile unsigned char temp;
    for (i=0;i<4;i++)
         temp = (unsigned char) fgetc(in);
    decompress_chars(in, out, haffman_tree, last_node);
};
int main()
{
    //MessageBox(0, "12345", "a", MB_OK);
    FILE *in = fopen("in.txt","rb");
    FILE *out = fopen("out.txt","wb");
    char cmd;
    volatile char temp;
    cmd = (char) fgetc(in);
    temp = (char) fgetc(in);
    temp = (char) fgetc(in);
    switch (cmd)
    {
        case 'c': compress(in, out); break;
        case 'd': decompress(in, out); break;
    }
    fclose(in);
    fclose(out);
    return 0;
}
