/**
 * Calculates the load for each thread so work is balanced
 * USAGE: ./<exec> <gnuplot.dat> <nr of threads> <output file>
 * 
 * STIU CA E INCREDIBIl DE PROST SCRIS, DAR E FACut PE REPEZEALA
 **/
#include <stdio.h>
#include <stdlib.h>

typedef struct vect_struct {
    double *values;
    int allocated_size;
    int actual_size;
} *Vector;

typedef struct result {
    int begin;
    int end;
    double timezz;
} Result;

double read_plot(Vector time_values, FILE *input) {
    int line_nr;
    double za_time, sum = 0;
    while (fscanf(input, "%d%lf", &line_nr, &za_time) != EOF) {
        if (line_nr >= (time_values->allocated_size)) {
            time_values->allocated_size *= 2;
            time_values->values = realloc(time_values->values, sizeof(double) *
                 time_values->allocated_size);

            if (!time_values->values) {
                fprintf(stderr, "realloc failed\n");
                exit(1);
            }
        }
        time_values->actual_size++;
        (time_values->values)[time_values->actual_size - 1] = za_time;
        sum += za_time;
    }
    return sum;
}

void permute_into_first(Result *r1, Result *r2, Vector time_values) {
    r1->timezz += time_values->values[r2->begin];
    r2->timezz -= time_values->values[r2->begin];
    r1->end++;
    r2->begin++;
}

void permute_into_second(Result *r1, Result *r2, Vector time_values) {
    r1->timezz -= time_values->values[r2->begin - 1];
    r2->timezz += time_values->values[r2->begin - 1];
    r1->end--;
    r2->begin--;
}

int size;
int nr_prt;
Result *results;
Result *solution;
Vector time_values;
double maximum;
double tpt;

int check(int k) {
    double sum = 0;
    for (int i = results[k].begin; i < results[k].end; i++) {
        sum += time_values->values[i];
    }
    if (sum >  maximum) return 0;
    if (sum < tpt * 8/ 10) return 0;
    return 1;
}

void compute_max() {
    double sum = 0;
    double ze_max = 0;
    for (int k = 0; k <= nr_prt; k++) {
        sum = 0;
        for (int i = results[k].begin; i < results[k].end; i++) {
            sum += time_values->values[i];
        }
        results[k].timezz = sum;
        if (sum > ze_max) {
            ze_max = sum;
        }
    }
    if (ze_max < maximum) {
        for (int i = 0; i <= nr_prt; i++) {
            solution[i].begin = results[i].begin;
            solution[i].end = results[i].end;
             solution[i].timezz = results[i].timezz;
            maximum = ze_max;
        }
    }
}

void bkt(int k) {
    if (k > nr_prt) {
        compute_max();
    } else {
        if (k > 0) {

            for (int i = results[k - 1].end + 2; i < size; i++) {
                if (k == nr_prt) {
                    results[k].begin = results[k - 1].end + 1;
                    results[k].end = size - 1;
                    if (check(k))
                        bkt(k + 1);
                } else {
                    results[k].begin = results[k - 1].end + 1;
                    results[k].end = i;
                    if (check(k))
                        bkt(k + 1);
                }
                
            }
        } else {
            for (int i = 1; i < size; i++) {
                results[k].begin = 0;
                results[k].end = i;
                if (check(k))
                    bkt(k + 1);
                
            }
        }
    }
}

int main(int argc, char **argv) {

    if (argc < 4) {
        fprintf(stderr, "Usage: %s <plot results file> <nr of threads> <output file>\n", argv[0]);
        exit(1);
    }
    FILE *plot_results, *output;
    plot_results = fopen(argv[1], "r");
    if (!plot_results) {
        fprintf(stderr, "Could not open %s file\n", argv[1]);
        exit(1);
    }
    output = fopen(argv[3], "w+");
    if (!output) {
        fprintf(stderr, "Could not open %s file\n", argv[3]);
        exit(1);
    }

    time_values = malloc(sizeof(struct vect_struct));
    if (!time_values) {
        fprintf(stderr, "malloc error :(\n");
    }
    (*time_values) = (struct vect_struct){
        .values = malloc(100 * sizeof(double)),
        .allocated_size = 100,
        .actual_size = 0,
    };

    if (!time_values->values) {
        fprintf(stderr, "malloc error :(\n");
        exit(1);
    } 

    double whole_time, time_per_thread;
    whole_time = read_plot(time_values, plot_results);
    time_per_thread = whole_time / atoi(argv[2]);
    fclose(plot_results);

    printf("'whole time' and 'time per thread': %lf %lf\n", whole_time, time_per_thread);
    double sum = 0;
    int begin, end, nr_prints = 0;
    begin = 0;
    Result result[time_values->actual_size];
    maximum = whole_time;

    size = time_values->actual_size;
    results = malloc(size * sizeof(Result));
    solution = malloc(size * sizeof(Result));


    for (int i = 0; i < time_values->actual_size; i++) {
        sum += time_values->values[i];

        if (sum > time_per_thread && nr_prints < atoi(argv[2]) - 1) {
            nr_prints++;
            end = i;
            result[nr_prints - 1].begin = begin;
            result[nr_prints - 1].end = end;
            result[nr_prints - 1].timezz = sum;
            //fprintf(output, "%d %d\n", begin, end);
            begin = i + 1;
            sum = 0;
        }
    }
    nr_prt = nr_prints;
    tpt = time_per_thread;

    result[nr_prints].begin = begin;
    result[nr_prints].end =  time_values->actual_size - 1;
    result[nr_prints].timezz = sum;
    //fprintf(output, "%d %d\n", begin, time_values->actual_size - 1);
    //TODO incercam prin bkt
    bkt(0);
    // for (int i = 0; i < size; i++) {
    //     results[i] = result[i];
    // }
    for (int i = 0; i <= nr_prints; i++) {
        //fprintf(output, "%d %d %lf\n", result[i].begin, result[i].end, result[i].timezz);
        //fprintf(output, "%d %d %lf\n", solution[i].begin, solution[i].end, solution[i].timezz);
        //fprintf(output, "%d %d\n", result[i].begin, result[i].end);
        fprintf(output, "%d %d\n", solution[i].begin, solution[i].end);
    }
    fclose(output);
    //free everyone
    free(results);
    free(solution);

    return 0;
}