/*-
 * Copyright (C) 2010, Audrey Diacre.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * $Id: application.c 583M 2012-03-02 12:24:07Z (local) $
 */

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ucard.h>

#include "marchant.h"
#include "on_password_requested.h"

#include "time.h"

static int      on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application);

int
main (void)
{
    struct kiosk *kiosk = kiosk_new ();
    struct ucard_application *application = marchant_application_new (on_password_requested);

    if (kiosk_devices_scan (kiosk)) {
    kiosk_setup (kiosk, application, on_card_presented);
    kiosk_start (kiosk);
    kiosk_wait (kiosk, NULL);
    kiosk_stop (kiosk);
    }
    kiosk_free (kiosk);

    ucard_application_free (application);

    exit (EXIT_SUCCESS);
}



static int
on_card_presented (struct ucard *ucard, struct ucard_application *ucard_application)
{
    int32_t balanceValue;
    int32_t amount = 0;
    char buffer[BUFSIZ];
    char lastuse[11];
    char date[11];
    time_t lastUse;
    time_t todayTime = time(NULL);
    struct tm *tmp;
    struct tm t1,t2;
    tmp = localtime(&todayTime);
    double deltaTime;

    // Read the last use of card
    marchant_date_read_data(ucard,ucard_application,0,0,lastuse);

    // If it's the first use, today's date
    if (lastuse[0] == '\0')
    {
        printf ("Welcome \n");
        strftime(lastuse, sizeof(lastuse),"%Y-%m-%d",tmp);
        marchant_date_write_data(ucard,ucard_application,0,sizeof(lastuse),lastuse);
    }  else
    {
        printf ("Last use: %s \n", lastuse);

        strptime(lastuse,"%Y-%m-%d",&t1);
        strftime(date, sizeof(date),"%Y-%m-%d",tmp);
        strptime(date,"%Y-%m-%d",&t2);
        todayTime = mktime(&t2);
        lastUse = mktime(&t1);
        deltaTime = difftime(todayTime,lastUse);

        // The card was used ther is more a year
        if (deltaTime > 31536000)
        {
            if (marchant_reduction_get_value (ucard, ucard_application, &balanceValue) < 0) {
		ucard_perror (ucard, "marchant_reduction_get_value");
		goto error;
	    }
            if (balanceValue > 0)
            {
                if (marchant_reduction_debit (ucard, ucard_application, balanceValue) < 0) {
		    ucard_perror (ucard, "marchant_reduction_debit");
		    goto error;
		}
                printf("RESET");
                strftime(lastuse, sizeof(lastuse),"%Y-%m-%d",tmp);
                marchant_date_write_data(ucard,ucard_application,0,sizeof(lastuse),lastuse);
                if (ucard_transaction_commit (ucard) < 0) {
		    ucard_perror (ucard, "ucard_transaction_commit");
		    goto error;
		}
                if (marchant_reduction_get_value (ucard, ucard_application, &balanceValue) < 0) {
		    ucard_perror (ucard, "marchant_reduction_get_value");
		    goto error;
		}
                printf ("Current balance : %d \n",balanceValue);
            }
        }
    }

    if (marchant_reduction_get_value (ucard, ucard_application, &balanceValue) < 0) {
	ucard_perror (ucard, "marchant_reduction_get_value");
	goto error;
    }

    //Purpose to use balance
    if (balanceValue > 0)
    {
        printf ("Old balance : %d, Do you want to use it?[yN]",balanceValue);

        if (fgets (buffer, BUFSIZ, stdin) == NULL){
            printf ("fgets failed: %s\n",strerror(errno));
        }
        bool use_balance = ((buffer[0] == 'y') || (buffer[0] == 'Y'));

        if (use_balance)
        {
            while ((amount==0)||(amount > balanceValue))
            {
                printf ("Amount : ");
                if (scanf("%d",&amount)< 1){
                    printf("scanf failed:%s\n",strerror(errno));
                }
            }
            if (marchant_reduction_debit (ucard, ucard_application, amount) < 0) {
		ucard_perror (ucard, "marchant_reduction_debit");
		goto error;
	    }
            if (ucard_transaction_commit (ucard) < 0) {
		ucard_perror (ucard, "ucard_transaction_commit");
		goto error;
	    }
            if (marchant_reduction_get_value (ucard, ucard_application, &balanceValue) < 0) {
		ucard_perror (ucard, "marchant_reduction_get_value");
		goto error;
	    }
            printf ("Current balance : %d \n",balanceValue);
        }
    }
    strftime(lastuse, sizeof(lastuse),"%Y-%m-%d",tmp);
    strptime(lastuse,"%Y-%m-%d",&t2);
    if (t2.tm_wday == 5)
    {
        printf ("Points are doubled \n");
        if (marchant_reduction_credit (ucard, ucard_application, 2) < 0) {
	    ucard_perror (ucard, "marchant_reduction_credit");
	    goto error;
	}
    } else
    {
        if (marchant_reduction_credit (ucard, ucard_application, 1) < 0) {
	    ucard_perror (ucard, "marchant_reduction_credit");
	    goto error;
	}
    }
    marchant_date_write_data(ucard,ucard_application,0,sizeof(lastuse),lastuse);
    if (ucard_transaction_commit (ucard) < 0) {
	ucard_perror (ucard, "ucard_transaction_commit");
	goto error;
    }
    if (marchant_reduction_get_value (ucard, ucard_application, &balanceValue) < 0) {
	ucard_perror (ucard, "marchant_reduction_get_value");
	goto error;
    }
    printf ("New balance : %d \n",balanceValue);

    return 1;

error:
    ucard_transaction_abort (ucard);

    return 0;
}

