input metauml;
outputtemplate:="%j-%3c.ps";
prologues := 3; 

Class.UCardApplicationKeys("UCard Application Keys")(
		"#count",
		"#keys"
		)();
Class.UCardApplicationFiles("UCard Application Files")(
		"#count",
		)();
Class.UCardApplicationFile("UCard Application File")(
		"#type",
		"#access_rights",
		"#properties"
		)();
Class.UCardApplication("UCard Application")(
		"#aid",
		)();

beginfig(1);
		Class.K("Kiosk")(
				"-max_fd",
				"-device_count"
			)(
				"+add_device()",
				"+devices_scan()",
				"+free()",
				"+setup()",
				"+start()",
				"+stop()",
				"+wait()",
				"-kiosk_run()"
			);
		Class.D("Device")(
				"-fds[2]",
				"-nfc_device_desc",
				"-thread"
			)(
				"+setup()"
			);
		Class.UCard("UCard")(
				"#tag",
				"-selected_aid",
				"-current_key",
				"-transaction_in_progress"
			)(
				"+authenticate()",
				"+read_data()",
				"+write_data()",
				"+read_records()",
				"+write_records()",
				"+clear_record_file()",
				"+value_file_get_value()",
				"+value_file_credit()",
				"+value_file_limited_credit()",
				"+value_file_debit()",
				"+transaction_commit()",
				"+transaction_rollback()"
			);
		K.e = D.w + (-30, 0);
		D.e = UCard.w + (-30, 0);
    drawObjects(K, D, UCard);
		link(association)(K.e -- D.w);
		link(association)(D.e -- UCard.w);
endfig;

beginfig(2);
	UCardApplication.sw = UCardApplicationKeys.ne + (-30, 30);
	UCardApplication.se = UCardApplicationFiles.nw + (30, 30);
	UCardApplicationFile.n = UCardApplicationFiles.s + (0, -30);
	drawObjects (UCardApplication, UCardApplicationKeys, UCardApplicationFiles, UCardApplicationFile);
	link(composition)(UCardApplicationFile.n -- UCardApplicationFiles.s);
	link(aggregationUni)(pathManhattanY(UCardApplicationFiles.n, UCardApplication.e));
	link(aggregationUni)(pathManhattanY(UCardApplicationKeys.n, UCardApplication.w));
%	stepX:=20;
%	link(aggregation)(pathStepY(UCardApplicationKeys.n, UCardApplication.w, 20));
endfig;
end
