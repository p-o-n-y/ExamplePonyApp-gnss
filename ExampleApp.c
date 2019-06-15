#include <stdio.h>
#include <math.h>
#include "pony.h"

void read(void);
void write(void);
void calculate(void);
void timeStep(void);

int main()

{

	if ((
		pony_add_plugin(read)
		&& pony_add_plugin(calculate)
		&& pony_add_plugin(write)
		&& pony_add_plugin(timeStep)
		))
	{
		if (pony_init("{gnss: {gps: in = \"log0101b (1).txt\" out = \"result.txt\"}}"))
		{
			while (pony_step());
		}
	}
	return 0;

}

static double  kepler(double e, double Mk)
{
	double Ek, temp_Ek = Mk;

	for (int i = 1; i < 8; i++)
	{
		Ek = e * sin(temp_Ek) + Mk;
		temp_Ek = Ek;
	}
	return Ek;
}

void DtoE(char* string)
{
	while (*string != '\0')
	{
		if (*string == 'D')
		{
			*string = 'E';
		}
		string++;
	}
}

void read(void)
{
	static FILE *f;

	if (pony.bus.mode > 0)
	{

	}

	else if (pony.bus.mode == 0)
	{
		char* fileName;
		int len;

		if (pony_extract_string_length(pony.bus.gnss->gps->cfg, pony.bus.gnss->gps->cfglength, "in = \"", &len))
		{
			fileName = malloc(sizeof(char*) * len + 1);
			pony_extract_string(pony.bus.gnss->gps->cfg, pony.bus.gnss->gps->cfglength, "in = \"", &fileName);
			fileName[len] = '\0';
		}
		else
		{
			fileName = "dataIn.txt";
		}

		f = fopen(fileName, "r");

		char buffer[1024];
		int scanned;
		int Satel_numb;
		double* ephval;
		double	*Crs,		/*meters*/
			*Del_n,		/*radians/sec*/
			*M0,			/*radians*/
			*Cuc,		/*radians*/
			*e_Eccentr,
			*Cus,		/*radians*/
			*sqrt_A,		/*Square Root of the Semi-Major Axis*/
			*Toe,		/*Time of Ephemeris(sec of GPS week)*/
			*Cic,		/*radians*/
			*OMEGA,		/*radians*/
			*Cis,		/*radians*/
			*i0,			/*radians*/
			*Crc,		/*meters*/
			*omega,		/*radians*/
			*OMEGA_DOT,	/*radians/sec*/
			*I_DOT,		/*radians/sec*/
			*SV_accuracy,/*(meters)*/
			*SV_health;	/*(bits 17 - 22 w 3 sf 1)*/

		for (int i = 0; i < 6; i++)
		{
			fgets(buffer, 1023, f);
		}

		while (!feof(f))
		{
			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%i %*i %*i %*i %*i %*i %*lg %*lg %*lg %*lg", &Satel_numb);

			Satel_numb--;

			ephval = pony.bus.gnss->gps->sat[Satel_numb].eph.val;
			Crs = &(ephval[0]);
			Del_n = &(ephval[1]);
			M0 = &(ephval[2]);
			Cuc = &(ephval[3]);
			e_Eccentr = &(ephval[4]);
			Cus = &(ephval[5]);
			sqrt_A = &(ephval[6]);
			Toe = &(ephval[7]);
			Cic = &(ephval[8]);
			OMEGA = &(ephval[9]);
			Cis = &(ephval[10]);
			i0 = &(ephval[11]);
			Crc = &(ephval[12]);
			omega = &(ephval[13]);
			OMEGA_DOT = &(ephval[14]);
			I_DOT = &(ephval[15]);
			SV_accuracy = &(ephval[16]);
			SV_health = &(ephval[17]);

			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%*lg %lg %lg %lg", Crs, Del_n, M0);
			if (scanned < 3)
				break;

			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%lg %lg %lg %lg", Cuc, e_Eccentr, Cus, sqrt_A);
			if (scanned < 4)
				break;

			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%lg %lg %lg %lg", Toe, Cic, OMEGA, Cis);
			if (scanned < 4)
				break;

			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%lg %lg %lg %lg", i0, Crc, omega, OMEGA_DOT);
			if (scanned < 4)
				break;

			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%lg %*lg %*lg %*lg", I_DOT);
			if (scanned < 1)
				break;

			fgets(buffer, 1023, f);
			DtoE(buffer);
			scanned = sscanf(buffer, "%lg %lg %*lg %*lg", SV_accuracy, SV_health);
			if (scanned < 2)
				break;

			fgets(buffer, 1023, f);


			pony.bus.gnss->gps->sat[Satel_numb].eph.valid = 1;

		}

		fclose(f);

	}

	else
	{

	}

}

void write(void)
{
	static FILE *f1;

	if (pony.bus.mode > 0)
	{
		if ((int)(pony.bus.t.val) % 60 == 0)
		{
			for (int i = 0; i < pony.bus.gnss->gps->max_sat_num; i++)
			{
				fprintf(f1, "% 2i %i % +12.6f % +12.6f % +12.6f ", i + 1, pony.bus.gnss->gps->sat[i].xVal, pony.bus.gnss->gps->sat[i].x[0], pony.bus.gnss->gps->sat[i].x[1], pony.bus.gnss->gps->sat[i].x[2]);
			}
			fprintf(f1, "\n");
		}
	}

	else if (pony.bus.mode == 0)
	{
		char* fileName;
		int len;

		if (pony_extract_string_length(pony.bus.gnss->gps->cfg, pony.bus.gnss->gps->cfglength, "out = \"", &len))
		{
			fileName = malloc(sizeof(char*) * len + 1);
			pony_extract_string(pony.bus.gnss->gps->cfg, pony.bus.gnss->gps->cfglength, "out = \"", &fileName);
			fileName[len] = '\0';
		}
		else
		{
			fileName = "dataOut.txt";
		}

		f1 = fopen(fileName, "w");
	}

	else
	{
		fclose(f1);
	}
}

void calculate(void)
{
	double* ephval;
	double	*Crs,		/*meters*/
		*Del_n,		/*radians/sec*/
		*M0,			/*radians*/
		*Cuc,		/*radians*/
		*e_Eccentr,
		*Cus,		/*radians*/
		*sqrt_A,		/*Square Root of the Semi-Major Axis*/
		*Toe,		/*Time of Ephemeris(sec of GPS week)*/
		*Cic,		/*radians*/
		*OMEGA,		/*radians*/
		*Cis,		/*radians*/
		*i0,			/*radians*/
		*Crc,		/*meters*/
		*omega,		/*radians*/
		*OMEGA_DOT,	/*radians/sec*/
		*I_DOT,		/*radians/sec*/
		*SV_accuracy,/*(meters)*/
		*SV_health;	/*(bits 17 - 22 w 3 sf 1)*/
	double μ = 3.986005*pow(10, 14), OMEGA_DOT_e = 7.2921151467*pow(10, -5), *t = &(pony.bus.t.val), A, n0, tk, n, Mk, ηk, Ek, Φk, uk, rk, ik, Xk_derevat, Yk_derevat, OMEGA_k, Xk, Yk, Zk, δuk, δrk, δik;

	if (pony.bus.mode > 0)
	{
		for (int i = 0; i < pony.bus.gnss->gps->max_sat_num; i++)
		{
			if (pony.bus.gnss->gps->sat[i].eph.valid)
			{
				ephval = pony.bus.gnss->gps->sat[i].eph.val;
				Crs = &(ephval[0]);
				Del_n = &(ephval[1]);
				M0 = &(ephval[2]);
				Cuc = &(ephval[3]);
				e_Eccentr = &(ephval[4]);
				Cus = &(ephval[5]);
				sqrt_A = &(ephval[6]);
				Toe = &(ephval[7]);
				Cic = &(ephval[8]);
				OMEGA = &(ephval[9]);
				Cis = &(ephval[10]);
				i0 = &(ephval[11]);
				Crc = &(ephval[12]);
				omega = &(ephval[13]);
				OMEGA_DOT = &(ephval[14]);
				I_DOT = &(ephval[15]);
				SV_accuracy = &(ephval[16]);
				SV_health = &(ephval[17]);

				A = pow(*sqrt_A, 2);
				n0 = sqrt((μ / pow(A, 3)));
				tk = *t - *Toe;
				n = n0 + *Del_n;
				Mk = *M0 + n * tk;
				Ek = kepler(*e_Eccentr, Mk);

				ηk = atan2(sqrt(1 - pow(*e_Eccentr, 2))*sin(Ek), cos(Ek) - *e_Eccentr);

				Φk = ηk + *omega;

				δuk = (*Cus*sin(2 * Φk)) + (*Cus * cos(2 * Φk));
				δrk = (*Crs*sin(2 * Φk)) + (*Crs * cos(2 * Φk));
				δik = (*Cis*sin(2 * Φk)) + (*Cis * cos(2 * Φk));

				uk = Φk + δuk;
				rk = A * (1 - (*e_Eccentr*cos(Ek))) + δrk;
				ik = *i0 + δik + (*I_DOT*tk);

				Xk_derevat = rk * cos(uk);
				Yk_derevat = rk * sin(uk);

				OMEGA_k = *OMEGA + ((*OMEGA_DOT - OMEGA_DOT_e) * tk) - (OMEGA_DOT_e* *Toe);

				pony.bus.gnss->gps->sat[i].x[0] = (Xk_derevat*cos(OMEGA_k)) - (Yk_derevat*cos(ik)*sin(OMEGA_k));
				pony.bus.gnss->gps->sat[i].x[1] = (Xk_derevat*sin(OMEGA_k)) + (Yk_derevat*cos(ik)*cos(OMEGA_k));
				pony.bus.gnss->gps->sat[i].x[2] = Yk_derevat * sin(ik);

				pony.bus.gnss->gps->sat[i].xVal = 1;
			}
		}
	}

	else if (pony.bus.mode == 0)
	{

	}

	else
	{

	}
}

void timeStep(void)
{
	if (pony.bus.mode > 0)
	{
		(pony.bus.t.val)++;
		if (pony.bus.t.val >= 43200)
		{
			pony.bus.mode = -1;
		}
	}

	else if (pony.bus.mode == 0)
	{
		pony.bus.t.val = 0;
	}

	else
	{

	}
}