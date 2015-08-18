/**
 * Dieser Typ wurde in VisualAge erstellt.
 */
public class Configure extends com.sun.java.swing.JFrame implements java.awt.event.ActionListener {
	private com.sun.java.swing.JButton ivjAbortButton = null;
	private com.sun.java.swing.JPanel ivjJFrameContentPane = null;
	private com.sun.java.swing.JButton ivjSaveConfigButton = null;
	private com.sun.java.swing.JPanel ivjTL2Page = null;
	private com.sun.java.swing.JLabel ivjCompileModeLabel = null;
	private com.sun.java.swing.JPanel ivjJPanel1 = null;
	private com.sun.java.swing.JRadioButton ivjDebugRButton = null;
	private com.sun.java.swing.JRadioButton ivjOptimizeRButton = null;
	private com.sun.java.swing.ButtonGroup ivjCompileModeButtonGroup = null;
	private com.sun.java.swing.JLabel ivjTL2TracingLabel = null;
	private com.sun.java.swing.JCheckBox ivjTMDebugCBox = null;
	private com.sun.java.swing.JCheckBox ivjTSPDebugCBox = null;
	private com.sun.java.swing.JCheckBox ivjTVMDebugCBox = null;
	private com.sun.java.swing.JLabel ivjVMTestLabel = null;
	private com.sun.java.swing.JPanel ivjVMTestPanel = null;
	private com.sun.java.swing.JPanel ivjTycoonTracingPanel = null;
	private com.sun.java.swing.JLabel ivjDebugFlagsLabel = null;
	private com.sun.java.swing.JLabel ivjMainPathLabel1 = null;
	private com.sun.java.swing.JPanel ivjPathPage1 = null;
	private com.sun.java.swing.JCheckBox ivjtosConditionCB = null;
	private com.sun.java.swing.JCheckBox ivjtosDllCB = null;
	private com.sun.java.swing.JCheckBox ivjtosFilenameCB = null;
	private com.sun.java.swing.JCheckBox ivjtosMutex = null;
	private com.sun.java.swing.JCheckBox ivjtosThreadCB = null;
	private com.sun.java.swing.JCheckBox ivjtosTLSCB = null;
	private com.sun.java.swing.JPanel ivjTycoonOSPage = null;
	private com.sun.java.swing.JTabbedPane ivjTycoonOSPane = null;
	private com.sun.java.swing.JCheckBox ivjtosProcessCB = null;
/**
 * Konstruktor
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
public Configure() {
	super();
	initialize();
}
/**
 * Configure - Konstruktorkommentar.
 * @param title java.lang.String
 */
public Configure(String title) {
	super(title);
}
/**
 * Methode für die Bearbeitung von Ereignissen für die Schnittstelle ActionListener.
 * @param e java.awt.event.ActionEvent
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
public void actionPerformed(java.awt.event.ActionEvent e) {
	// Benutzercode - Anfang {1}
	// Benutzercode - Ende
	if ((e.getSource() == getAbortButton()) ) {
		connEtoM1(e);
	}
	// Benutzercode - Anfang {2}
	// Benutzercode - Ende
}
/**
 * connEtoM1:  (AbortButton.action.actionPerformed(java.awt.event.ActionEvent) --> Configure.dispose()V)
 * @param arg1 java.awt.event.ActionEvent
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private void connEtoM1(java.awt.event.ActionEvent arg1) {
	try {
		// Benutzercode - Anfang {1}
		// Benutzercode - Ende
		this.dispose();
		// Benutzercode - Anfang {2}
		// Benutzercode - Ende
	} catch (java.lang.Throwable ivjExc) {
		// Benutzercode - Anfang {3}
		// Benutzercode - Ende
		handleException(ivjExc);
	}
}
/**
 * Den Eigenschaftswert AbortButton zurückgeben.
 * @return com.sun.java.swing.JButton
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JButton getAbortButton() {
	if (ivjAbortButton == null) {
		try {
			ivjAbortButton = new com.sun.java.swing.JButton();
			ivjAbortButton.setName("AbortButton");
			ivjAbortButton.setText("Abort Configuration");
			ivjAbortButton.setBounds(334, 234, 152, 25);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjAbortButton;
}
/**
 * Den Eigenschaftswert CompileModeButtonGroup zurückgeben.
 * @return com.sun.java.swing.ButtonGroup
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.ButtonGroup getCompileModeButtonGroup() {
	if (ivjCompileModeButtonGroup == null) {
		try {
			ivjCompileModeButtonGroup = new com.sun.java.swing.ButtonGroup();
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjCompileModeButtonGroup;
}
/**
 * Den Eigenschaftswert CompileModeLabel zurückgeben.
 * @return com.sun.java.swing.JLabel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JLabel getCompileModeLabel() {
	if (ivjCompileModeLabel == null) {
		try {
			ivjCompileModeLabel = new com.sun.java.swing.JLabel();
			ivjCompileModeLabel.setName("CompileModeLabel");
			ivjCompileModeLabel.setText("Compile mode:");
			ivjCompileModeLabel.setBounds(11, 7, 92, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjCompileModeLabel;
}
/**
 * Den Eigenschaftswert DebugFlagsLabel zurückgeben.
 * @return com.sun.java.swing.JLabel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JLabel getDebugFlagsLabel() {
	if (ivjDebugFlagsLabel == null) {
		try {
			ivjDebugFlagsLabel = new com.sun.java.swing.JLabel();
			ivjDebugFlagsLabel.setName("DebugFlagsLabel");
			ivjDebugFlagsLabel.setText("TycoonOS debug flags:");
			ivjDebugFlagsLabel.setBounds(16, 17, 155, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjDebugFlagsLabel;
}
/**
 * Den Eigenschaftswert JRadioButton1 zurückgeben.
 * @return com.sun.java.swing.JRadioButton
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JRadioButton getDebugRButton() {
	if (ivjDebugRButton == null) {
		try {
			ivjDebugRButton = new com.sun.java.swing.JRadioButton();
			ivjDebugRButton.setName("DebugRButton");
			ivjDebugRButton.setToolTipText("Activate compiler flags for generating debug informations");
			ivjDebugRButton.setText("Debug Flags");
			ivjDebugRButton.setBounds(11, 29, 110, 15);
			// Benutzercode - Anfang {1}
			getCompileModeButtonGroup().add(ivjDebugRButton);
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjDebugRButton;
}
/**
 * Den Eigenschaftswert JFrameContentPane zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getJFrameContentPane() {
	if (ivjJFrameContentPane == null) {
		try {
			ivjJFrameContentPane = new com.sun.java.swing.JPanel();
			ivjJFrameContentPane.setName("JFrameContentPane");
			ivjJFrameContentPane.setLayout(null);
			getJFrameContentPane().add(getSaveConfigButton(), getSaveConfigButton().getName());
			getJFrameContentPane().add(getAbortButton(), getAbortButton().getName());
			getJFrameContentPane().add(getTycoonOSPane(), getTycoonOSPane().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjJFrameContentPane;
}
/**
 * Den Eigenschaftswert JPanel1 zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getJPanel1() {
	if (ivjJPanel1 == null) {
		try {
			ivjJPanel1 = new com.sun.java.swing.JPanel();
			ivjJPanel1.setName("JPanel1");
			ivjJPanel1.setBorder(new com.sun.java.swing.plaf.basic.BasicFieldBorder());
			ivjJPanel1.setLayout(null);
			ivjJPanel1.setBounds(13, 16, 122, 74);
			getJPanel1().add(getCompileModeLabel(), getCompileModeLabel().getName());
			getJPanel1().add(getDebugRButton(), getDebugRButton().getName());
			getJPanel1().add(getOptimizeRButton(), getOptimizeRButton().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjJPanel1;
}
/**
 * Den Eigenschaftswert MainPathLabel1 zurückgeben.
 * @return com.sun.java.swing.JLabel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JLabel getMainPathLabel1() {
	if (ivjMainPathLabel1 == null) {
		try {
			ivjMainPathLabel1 = new com.sun.java.swing.JLabel();
			ivjMainPathLabel1.setName("MainPathLabel1");
			ivjMainPathLabel1.setText("Main TL-2 path");
			ivjMainPathLabel1.setBounds(18, 26, 93, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjMainPathLabel1;
}
/**
 * Den Eigenschaftswert JRadioButton2 zurückgeben.
 * @return com.sun.java.swing.JRadioButton
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JRadioButton getOptimizeRButton() {
	if (ivjOptimizeRButton == null) {
		try {
			ivjOptimizeRButton = new com.sun.java.swing.JRadioButton();
			ivjOptimizeRButton.setName("OptimizeRButton");
			ivjOptimizeRButton.setToolTipText("Activate optimizing compiler flags");
			ivjOptimizeRButton.setText("Optimize code");
			ivjOptimizeRButton.setBounds(11, 51, 110, 15);
			// Benutzercode - Anfang {1}
			getCompileModeButtonGroup().add(ivjOptimizeRButton);			
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjOptimizeRButton;
}
/**
 * Den Eigenschaftswert PathPage1 zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getPathPage1() {
	if (ivjPathPage1 == null) {
		try {
			ivjPathPage1 = new com.sun.java.swing.JPanel();
			ivjPathPage1.setName("PathPage1");
			ivjPathPage1.setLayout(null);
			getPathPage1().add(getMainPathLabel1(), getMainPathLabel1().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjPathPage1;
}
/**
 * Den Eigenschaftswert SaveConfigButton zurückgeben.
 * @return com.sun.java.swing.JButton
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JButton getSaveConfigButton() {
	if (ivjSaveConfigButton == null) {
		try {
			ivjSaveConfigButton = new com.sun.java.swing.JButton();
			ivjSaveConfigButton.setName("SaveConfigButton");
			ivjSaveConfigButton.setText("Save Configuration");
			ivjSaveConfigButton.setBounds(334, 197, 152, 25);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjSaveConfigButton;
}
/**
 * Den Eigenschaftswert TL2Page zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getTL2Page() {
	if (ivjTL2Page == null) {
		try {
			ivjTL2Page = new com.sun.java.swing.JPanel();
			ivjTL2Page.setName("TL2Page");
			ivjTL2Page.setLayout(null);
			getTL2Page().add(getJPanel1(), getJPanel1().getName());
			getTL2Page().add(getVMTestPanel(), getVMTestPanel().getName());
			getTL2Page().add(getTycoonTracingPanel(), getTycoonTracingPanel().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTL2Page;
}
/**
 * Den Eigenschaftswert TL2TracingLabel zurückgeben.
 * @return com.sun.java.swing.JLabel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JLabel getTL2TracingLabel() {
	if (ivjTL2TracingLabel == null) {
		try {
			ivjTL2TracingLabel = new com.sun.java.swing.JLabel();
			ivjTL2TracingLabel.setName("TL2TracingLabel");
			ivjTL2TracingLabel.setText("Tycoon-2 Tracing:");
			ivjTL2TracingLabel.setBounds(11, 12, 134, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTL2TracingLabel;
}
/**
 * Den Eigenschaftswert TMDebugCBox zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox getTMDebugCBox() {
	if (ivjTMDebugCBox == null) {
		try {
			ivjTMDebugCBox = new com.sun.java.swing.JCheckBox();
			ivjTMDebugCBox.setName("TMDebugCBox");
			ivjTMDebugCBox.setToolTipText("Compile test mode of tmdebug.c");
			ivjTMDebugCBox.setText("tmdebug.c");
			ivjTMDebugCBox.setBounds(12, 27, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTMDebugCBox;
}
/**
 * Den Eigenschaftswert tosConditionCB zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosConditionCB() {
	if (ivjtosConditionCB == null) {
		try {
			ivjtosConditionCB = new com.sun.java.swing.JCheckBox();
			ivjtosConditionCB.setName("tosConditionCB");
			ivjtosConditionCB.setText("tosCondition");
			ivjtosConditionCB.setBounds(158, 65, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosConditionCB;
}
/**
 * Den Eigenschaftswert tosDllCB zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosDllCB() {
	if (ivjtosDllCB == null) {
		try {
			ivjtosDllCB = new com.sun.java.swing.JCheckBox();
			ivjtosDllCB.setName("tosDllCB");
			ivjtosDllCB.setText("tosDll");
			ivjtosDllCB.setBounds(158, 177, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosDllCB;
}
/**
 * Den Eigenschaftswert tosFilenameCB zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosFilenameCB() {
	if (ivjtosFilenameCB == null) {
		try {
			ivjtosFilenameCB = new com.sun.java.swing.JCheckBox();
			ivjtosFilenameCB.setName("tosFilenameCB");
			ivjtosFilenameCB.setText("tosFilename");
			ivjtosFilenameCB.setBounds(17, 38, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosFilenameCB;
}
/**
 * Den Eigenschaftswert tosMutex zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosMutex() {
	if (ivjtosMutex == null) {
		try {
			ivjtosMutex = new com.sun.java.swing.JCheckBox();
			ivjtosMutex.setName("tosMutex");
			ivjtosMutex.setText("tosMutex");
			ivjtosMutex.setBounds(158, 91, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosMutex;
}
/**
 * Den Eigenschaftswert tosProcessCB zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosProcessCB() {
	if (ivjtosProcessCB == null) {
		try {
			ivjtosProcessCB = new com.sun.java.swing.JCheckBox();
			ivjtosProcessCB.setName("tosProcessCB");
			ivjtosProcessCB.setText("tosProcess");
			ivjtosProcessCB.setBounds(158, 155, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosProcessCB;
}
/**
 * Den Eigenschaftswert tosThreadCB zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosThreadCB() {
	if (ivjtosThreadCB == null) {
		try {
			ivjtosThreadCB = new com.sun.java.swing.JCheckBox();
			ivjtosThreadCB.setName("tosThreadCB");
			ivjtosThreadCB.setText("tosThread");
			ivjtosThreadCB.setBounds(158, 39, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosThreadCB;
}
/**
 * Den Eigenschaftswert tosTLSCB zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox gettosTLSCB() {
	if (ivjtosTLSCB == null) {
		try {
			ivjtosTLSCB = new com.sun.java.swing.JCheckBox();
			ivjtosTLSCB.setName("tosTLSCB");
			ivjtosTLSCB.setText("tosTLS");
			ivjtosTLSCB.setBounds(158, 118, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjtosTLSCB;
}
/**
 * Den Eigenschaftswert TSPDebugCBox zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox getTSPDebugCBox() {
	if (ivjTSPDebugCBox == null) {
		try {
			ivjTSPDebugCBox = new com.sun.java.swing.JCheckBox();
			ivjTSPDebugCBox.setName("TSPDebugCBox");
			ivjTSPDebugCBox.setText("TSP Debug");
			ivjTSPDebugCBox.setBounds(11, 57, 134, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTSPDebugCBox;
}
/**
 * Den Eigenschaftswert TVMDebugCBox zurückgeben.
 * @return com.sun.java.swing.JCheckBox
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JCheckBox getTVMDebugCBox() {
	if (ivjTVMDebugCBox == null) {
		try {
			ivjTVMDebugCBox = new com.sun.java.swing.JCheckBox();
			ivjTVMDebugCBox.setName("TVMDebugCBox");
			ivjTVMDebugCBox.setText("TVM Debug");
			ivjTVMDebugCBox.setBounds(11, 36, 134, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTVMDebugCBox;
}
/**
 * Den Eigenschaftswert PathPage zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getTycoonOSPage() {
	if (ivjTycoonOSPage == null) {
		try {
			ivjTycoonOSPage = new com.sun.java.swing.JPanel();
			ivjTycoonOSPage.setName("TycoonOSPage");
			ivjTycoonOSPage.setLayout(null);
			getTycoonOSPage().add(gettosFilenameCB(), gettosFilenameCB().getName());
			getTycoonOSPage().add(getDebugFlagsLabel(), getDebugFlagsLabel().getName());
			getTycoonOSPage().add(gettosDllCB(), gettosDllCB().getName());
			getTycoonOSPage().add(gettosTLSCB(), gettosTLSCB().getName());
			getTycoonOSPage().add(gettosMutex(), gettosMutex().getName());
			getTycoonOSPage().add(gettosConditionCB(), gettosConditionCB().getName());
			getTycoonOSPage().add(gettosThreadCB(), gettosThreadCB().getName());
			getTycoonOSPage().add(gettosProcessCB(), gettosProcessCB().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTycoonOSPage;
}
/**
 * Den Eigenschaftswert JTabbedPane zurückgeben.
 * @return com.sun.java.swing.JTabbedPane
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JTabbedPane getTycoonOSPane() {
	if (ivjTycoonOSPane == null) {
		try {
			ivjTycoonOSPane = new com.sun.java.swing.JTabbedPane();
			ivjTycoonOSPane.setName("TycoonOSPane");
			ivjTycoonOSPane.setBounds(17, 15, 306, 247);
			ivjTycoonOSPane.insertTab("TL-2", null, getTL2Page(), null, 0);
			ivjTycoonOSPane.insertTab("TycoonOS", null, getTycoonOSPage(), null, 1);
			ivjTycoonOSPane.insertTab("Directories", null, getPathPage1(), null, 2);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTycoonOSPane;
}
/**
 * Den Eigenschaftswert TycoonTracingPanel zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getTycoonTracingPanel() {
	if (ivjTycoonTracingPanel == null) {
		try {
			ivjTycoonTracingPanel = new com.sun.java.swing.JPanel();
			ivjTycoonTracingPanel.setName("TycoonTracingPanel");
			ivjTycoonTracingPanel.setBorder(new com.sun.java.swing.plaf.basic.BasicFieldBorder());
			ivjTycoonTracingPanel.setLayout(null);
			ivjTycoonTracingPanel.setBounds(14, 103, 263, 105);
			getTycoonTracingPanel().add(getTL2TracingLabel(), getTL2TracingLabel().getName());
			getTycoonTracingPanel().add(getTVMDebugCBox(), getTVMDebugCBox().getName());
			getTycoonTracingPanel().add(getTSPDebugCBox(), getTSPDebugCBox().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjTycoonTracingPanel;
}
/**
 * Den Eigenschaftswert VMTestLabel zurückgeben.
 * @return com.sun.java.swing.JLabel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JLabel getVMTestLabel() {
	if (ivjVMTestLabel == null) {
		try {
			ivjVMTestLabel = new com.sun.java.swing.JLabel();
			ivjVMTestLabel.setName("VMTestLabel");
			ivjVMTestLabel.setText("VM Test:");
			ivjVMTestLabel.setBounds(12, 6, 99, 15);
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjVMTestLabel;
}
/**
 * Den Eigenschaftswert VMTestPanel zurückgeben.
 * @return com.sun.java.swing.JPanel
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private com.sun.java.swing.JPanel getVMTestPanel() {
	if (ivjVMTestPanel == null) {
		try {
			ivjVMTestPanel = new com.sun.java.swing.JPanel();
			ivjVMTestPanel.setName("VMTestPanel");
			ivjVMTestPanel.setBorder(new com.sun.java.swing.plaf.basic.BasicFieldBorder());
			ivjVMTestPanel.setLayout(null);
			ivjVMTestPanel.setBounds(154, 17, 122, 74);
			getVMTestPanel().add(getVMTestLabel(), getVMTestLabel().getName());
			getVMTestPanel().add(getTMDebugCBox(), getTMDebugCBox().getName());
			// Benutzercode - Anfang {1}
			// Benutzercode - Ende
		} catch (java.lang.Throwable ivjExc) {
			// Benutzercode - Anfang {2}
			// Benutzercode - Ende
			handleException(ivjExc);
		}
	};
	return ivjVMTestPanel;
}
/**
 * Wird aufgerufen, wenn die Komponente eine Ausnahmebedingung übergibt.
 * @param exception java.lang.Throwable
 */
private void handleException(Throwable exception) {

	/* Entfernen die den Kommentar für die folgenden Zeilen, um nicht abgefangene Ausnahmebedingungen auf der Standardausgabeeinheit (stdout) auszugeben */
	// System.out.println("--------- NICHT ABGEFANGENE AUSNAHMEBEDINGUNG ---------");
	// exception.printStackTrace(System.out);
}
/**
 * Initialisiert Verbindungen
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private void initConnections() {
	// Benutzercode - Anfang {1}
	// Benutzercode - Ende
	getAbortButton().addActionListener(this);
}
/**
 * Die Klasse initialisieren.
 */
/* WARNUNG: DIESE METHODE WIRD ERNEUT GENERIERT. */
private void initialize() {
	// Benutzercode - Anfang {1}
	// Benutzercode - Ende
	setName("Configure");
	setDefaultCloseOperation(com.sun.java.swing.WindowConstants.DISPOSE_ON_CLOSE);
	setSize(509, 286);
	setTitle("Tycoon-2 Configuration");
	setContentPane(getJFrameContentPane());
	initConnections();
	// Benutzercode - Anfang {2}
	// Benutzercode - Ende
}
/**
 * Haupteingangspunkt - Startet die Komponente, wenn sie als Anwendung ausgeführt wird
 * @param args java.lang.String[]
 */
public static void main(java.lang.String[] args) {
	try {
		Configure aConfigure;
		aConfigure = new Configure();
		try {
			Class aCloserClass = Class.forName("com.ibm.uvm.abt.edit.WindowCloser");
			Class parmTypes[] = { java.awt.Window.class };
			Object parms[] = { aConfigure };
			java.lang.reflect.Constructor aCtor = aCloserClass.getConstructor(parmTypes);
			aCtor.newInstance(parms);
		} catch (java.lang.Throwable exc) {};
		aConfigure.setVisible(true);
	} catch (Throwable exception) {
		System.err.println("In main() von com.sun.java.swing.JFrame trat eine Ausnahmebedingung auf");
		exception.printStackTrace(System.out);
	}
}
}
