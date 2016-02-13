/*
 * axp20x-calibrate-battery.c
 * Utility for changing Rdc and OCV values
 * in mainline kernel with axp20x sysfs patch
 * Tested on cubietruck
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include <sys/io.h>
#include <sys/types.h>
#include <dirent.h>
#include <error.h>
#include <errno.h>

#define VERSION "v1.0"

const char rdc_fname[] = "/sys/power/axp_pmu/control/battery_rdc";
const char fg_fname[] = "/sys/power/axp_pmu/control/disable_fuel_gauge";
const char ocv_fname[] = "/sys/power/axp_pmu/ocv_curve";
const char axp_dirname[] = "/sys/power/axp_pmu/";

/* Extracted from cubietruck.fex */
const float ocv_table[] = {
	3.1328,
	3.2736,
	3.4144,
	3.5552,
	3.6256,
	3.6608,
	3.6960,
	3.7312,
	3.7664,
	3.8016,
	3.8368,
	3.8720,
	3.9424,
	4.0128,
	4.0832,
	4.1536
};

void clear_screen()
{
	printf("\033[2J\033[1;1H");
}

void read_configuration(int *rdc, uint8_t *capacity)
{
	FILE *rdc_file = fopen(rdc_fname, "r");
	if (!rdc_file)
		error(-1, errno, "Error opening battery Rdc sysfs entry");
	int result = fscanf(rdc_file, "%d", rdc);
	if (result == EOF) {
		fclose(rdc_file);
		error(-1, errno, "Error reading battery Rdc sysfs entry");
	}
	fclose(rdc_file);
	FILE *ocv_file = fopen(ocv_fname, "rb");
	if (!ocv_file)
		error(-1, errno, "Error opening battery OCV sysfs entry");
	size_t result2 = fread(capacity, 1, 16, ocv_file);
	if (result2 < 16) {
		fclose(rdc_file);
		error(-1, errno, "Error reading battery OCV sysfs entry");
	}
	fclose(ocv_file);
}

void save_configuration(int rdc, const uint8_t capacity[])
{
	FILE *rdc_file = fopen(rdc_fname, "r+");
	if (!rdc_file)
		error(-1, errno, "Error opening battery Rdc sysfs entry");
	int result = fprintf(rdc_file, "%d", rdc);
	if (result < 0) {
		fclose(rdc_file);
		error(-1, errno, "Error writing to battery Rdc sysfs entry");
	}
	fclose(rdc_file);
	FILE *ocv_file = fopen(ocv_fname, "r+b");
	if (!ocv_file)
		error(-1, errno, "Error opening battery OCV sysfs entry");
	size_t result2 = fwrite(capacity, 1, 16, ocv_file);
	if (result2 < 16) {
		fclose(rdc_file);
		error(-1, errno, "Error writing to battery OCV sysfs entry");
	}
	fclose(ocv_file);
}

void set_fuel_gauge(bool state)
{
	FILE *fg_file = fopen(fg_fname, "r+");
	if (!fg_file)
		error(-1, errno, "Error opening battery fuel gauge control sysfs entry");
	int result = fprintf(fg_file, "%d", state ? 0 : 1);
	if (result < 0) {
		fclose(fg_file);
		error(-1, errno, "Error writing to battery fuel gauge control sysfs entry");
	}
	fclose(fg_file);
}

void print_configuration(int rdc, const uint8_t capacity[])
{
	printf("Rdc value is %d uOhm\n\n", rdc);
	printf("#\tOCV(V)\tCapacity(%%)\n");
	for (int i = 0; i < 16; i++)
		printf("%d\t%6.4f\t%11d\n", i, ocv_table[i], capacity[i]);
	printf("\n");
}

bool edit_configuration(int *rdc, uint8_t *capacity)
{
	bool result = false;
	int status = 0;
	char *line = NULL;
	size_t len, size;
	int n, num, val;
	do {
		clear_screen();
		if (status == -1)
			printf("Invalid input, data not changed\n");
		else if (status == 1)
			printf("OK\n");
		print_configuration(*rdc, capacity);
		status = -1;
		printf("Enter OCV number (0-15) or \"rdc\" to change, <none> to finish: ");
		__fpurge(stdin);
		if ((len = getline(&line, &size, stdin)) != -1) {
			if (!strcmp(line, "rdc\n")) {
				printf("Enter new Rdc (75000-1000000) [%d]: ", *rdc);
				n = scanf("%d", &val);
				if (n == 1 && val > 75000 && val < 1000000) {
					*rdc = val;
					status = 1;
				}
			} else {
				n = sscanf(line, "%d", &num);
				if (n == 1 && num >= 0 && num <= 15) {
					printf("Enter new capacity (0-100) [%d]: ", capacity[num]);
					n = scanf("%d", &val);
					if (n == 1 && val >= 0 && val <= 100) {
						capacity[num] = val;
						status = 1;
					}
				}
			}
			free(line);
			line = NULL;
			size = 0;
		}
	} while (len > 1);
	printf("Save changes? (Y/N) [N]: ");
	__fpurge(stdin);
	if ((len = getline(&line, &size, stdin)) != -1) {
		if (!strcmp(line, "Y\n"))
			result = true;
		free(line);
	}
	return result;
}

int main(int argc, char *argv[])
{
	printf("axp20x battery calibration tool " VERSION "\n\n");
	int c;
	bool do_edit = false;
	while ((c = getopt(argc, argv, "he")) != -1) {
		switch (c) {
		case 'h':
			printf("Usage: %s [-eh]\n", program_invocation_short_name);
			printf("Arguments:\n");
			printf("-e:\tEdit configuration\n");
			printf("-h:\tShow this help message and exit\n");
			return 0;
		case 'e':
			if(geteuid() != 0)
				error(-1, EACCES, "Please run as root to edit configuration");
			do_edit = true;
			break;
		default:
			error(-1, EINVAL, "Unknown option: %c", c);
		}
	}
	/* Check if sysfs directory exists */
	DIR* dir = opendir(axp_dirname);
	if (dir) {
    	closedir(dir);
	} else if (ENOENT == errno) {
    	error(-1, errno, "Unable to find sysfs directory");
	} else {
    	error(-1, errno, "Unable to check for axp20x sysfs directory");
	}
	int rdc;
	uint8_t capacity[16];
	read_configuration(&rdc, capacity);
	if (do_edit && edit_configuration(&rdc, capacity)) {
		printf("Saving configuration\n");
		set_fuel_gauge(false);
		save_configuration(rdc, capacity);
		set_fuel_gauge(true);
	}
	print_configuration(rdc, capacity);

	return 0;
}
