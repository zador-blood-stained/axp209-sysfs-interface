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

#define VERSION "v1.1"

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
		fclose(ocv_file);
		error(-1, errno, "Error reading battery OCV sysfs entry");
	}
	fclose(ocv_file);
}

void write_configuration(int rdc, const uint8_t capacity[])
{
	/* TODO: Always reenable fuel gauge on error */
	set_fuel_gauge(false);
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
		fclose(ocv_file);
		error(-1, errno, "Error writing to battery OCV sysfs entry");
	}
	fclose(ocv_file);
	set_fuel_gauge(true);
	printf("Write OCV table to sysfs: OK\n");
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

void import_configuration(char *filename, uint8_t *capacity)
{
	FILE *ocv_file = fopen(filename, "rb");
	if (!ocv_file)
		error(-1, errno, "Error opening OCV file");
	size_t result = fread(capacity, 1, 16, ocv_file);
	if (result < 16) {
		fclose(ocv_file);
		error(-1, errno, "Error reading OCV file");
	}
	fclose(ocv_file);
	printf("Import OCV table from %s: OK\n", filename);
}

void export_configuration(char *filename, const uint8_t capacity[])
{
	FILE *ocv_file = fopen(filename, "wb");
	if (!ocv_file)
		error(-1, errno, "Error creating OCV file");
	size_t result = fwrite(capacity, 1, 16, ocv_file);
	if (result < 16) {
		fclose(ocv_file);
		error(-1, errno, "Error writing to OCV file");
	}
	fclose(ocv_file);
	printf("Export OCV table to %s OK\n", filename);
}

void show_help()
{
	printf("Usage: %s [-h | -p | -e | -l <file> | -s <file>]\n", program_invocation_short_name);
	printf("Arguments:\n");
	printf("-h:\t\tShow this help message and exit\n");
	printf("-p:\t\tPrint configuration\n");
	printf("-e:\t\tEdit configuration\n");
	printf("-s <file>:\tSave OCV table to <file>\n");
	printf("-l <file>:\tLoad and apply OCV table from <file>\n");
}

int main(int argc, char *argv[])
{
	printf("axp20x battery calibration tool " VERSION "\n\n");
	/* Check if sysfs directory exists */
	DIR* dir = opendir(axp_dirname);
	if (dir) {
		closedir(dir);
	} else if (ENOENT == errno) {
		error(-1, errno, "Unable to find sysfs directory");
	} else {
		error(-1, errno, "Unable to check for axp20x sysfs directory");
	}
	int c;
	char *filename;
	int rdc;
	uint8_t capacity[16];
	/* Get options; only one option at a time is supported/needed */
	if ((c = getopt(argc, argv, "hpel:s:")) != -1) {
		switch (c) {
		case 'h':
			show_help();
			return 0;
		case 'p':
			read_configuration(&rdc, capacity);
			print_configuration(rdc, capacity);
			return 0;
		case 'e':
			if (geteuid() != 0)
				error(-1, EACCES, "Please run as root to edit configuration");
			read_configuration(&rdc, capacity);
			if (edit_configuration(&rdc, capacity))
				write_configuration(rdc, capacity);
			return 0;
		case 'l':
			if (geteuid() != 0)
				error(-1, EACCES, "Please run as root to edit configuration");
			filename = strdup(optarg);
			read_configuration(&rdc, capacity);
			import_configuration(filename, capacity);
			write_configuration(rdc, capacity);
			//if (access(filename, R_OK) != -1)
			//	error(-1, EIO, "Error opening file %s", filename);
			return 0;
		case 's':
			filename = strdup(optarg);
			read_configuration(&rdc, capacity);
			export_configuration(filename, capacity);
			return 0;
		default:
			error(-1, EINVAL, "Unknown option: %c", c);
		}
	} else {
		show_help();
		return 0;
	}
	return 0;
}
