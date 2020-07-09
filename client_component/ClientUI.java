

/**
 * This file is the client part that request camera to caputre stream of jpeg and send over TCP socket
 */
import java.awt.BorderLayout;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.plaf.synth.SynthSpinnerUI;

public class ClientUI extends JFrame implements Runnable, ActionListener {

	
	static String camip = "192.168.20.247"; // camera IP Adreess
	static int camport = 8082; // camera PortNumber
	static String defaultPort = "8082"; //Default port number
	static String defaultFPS = "20"; //Default FPS
	static boolean isFinished = false;

	static Socket socket = null; // socket
	static DataOutputStream dos = null; // Writes data
	static DataInputStream dis = null;// Reads data
	byte[] imagebyteArray;

	JFrame frame = new JFrame(); //Frame 
	JLabel label = new JLabel(); //label
	JPanel panel = new JPanel(new BorderLayout());
	

	static JTextField ipAddress = null;
	static JTextField portnumber = null;
	static JTextField fps = null;
	@SuppressWarnings("rawtypes")
	static JComboBox resolList = null;
	int selectedResolutionIndex = 0;
	int resWidth = 0;
	int resHeight = 0;
	Thread streamThread = null;

	static String[] resolStrings = { "Select Resolution", "1280x960", "1280x720", "1024x768", "1024x640", "640x480",
			"640x400", "320x240", "176x144" };

	RSA client, server;

	public ClientUI() {
		//  CONSTRUCTOR
	}


	public void run() {
		System.out.println("Thread has entered");
		try {
			this.receiveImage();
		} catch (IOException e) {
			e.printStackTrace();
			// close socket
			try {
				socket.close();
				System.out.println("socket is closed");
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
		}
		//close socket
		try {
			socket.close();
			System.out.println("socket is closed");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public static void createSocket() {
		System.out.println(" create Socket");
		// Create socket connection
		try {
			System.out.println("Cam ip: " + ipAddress.getText());
			System.out.println("Cam port is " + Integer.parseInt(portnumber.getText()));
			socket = new Socket(ipAddress.getText(), Integer.parseInt(portnumber.getText()));
		} catch (UnknownHostException e) {
			System.out.println("Unknown host: " + ipAddress.getSelectedText());
			System.exit(1);
		} catch (IOException e) {
			System.out.println("Exit program");
			System.exit(1);
		}
		System.out.println("Exit : Socket created");
	}

	public void receiveImage() throws IOException {
		
		isFinished = false;
		while (isFinished == false) {
			imagebyteArray = receiveEncByteArray();
			displayImage();
		} // end of while
		dis.close();
		dos.close();
	}

	public void displayImage() {
		// Displaying of image
		
		label.setIcon(new ImageIcon(imagebyteArray));
		frame.getContentPane().add(label, BorderLayout.CENTER);
		frame.setLocation(0, 225);
		frame.pack();
		frame.setTitle("Image Display");
		frame.setVisible(true);
		
	}

	public void createMainWindow() throws IOException {
		getContentPane().add(panel);
		panel.setLayout(null);
		setSize(600, 400);
		setTitle("User Interface");
		setDefaultCloseOperation(EXIT_ON_CLOSE);

		ipAddress = new JTextField();
		portnumber = new JTextField();
		fps = new JTextField();
        // //Label for Camera IP Address
		JLabel labelipAddress = new JLabel(" Camera's IP Address");
		labelipAddress.setBounds(12, 40, 167, 15);
		
		ipAddress.setBounds(200, 40, 114, 19);
		
		// //Label for Camera port number
		JLabel labelPort = new JLabel("Camera's Port Number");
		labelPort.setBounds(12, 90, 167, 15);
		portnumber.setBounds(200, 90, 100, 19);
		
         //Label for Frequency
		JLabel labelFrequency = new JLabel("Frequency");
		labelFrequency.setBounds(12, 140, 156, 15);
		fps.setBounds(200, 125, 100, 25);
		labelFrequency.setLabelFor(fps);
		
	   ipAddress.setText(camip);
		portnumber.setText(defaultPort);
		fps.setText(defaultFPS);

		
		
		JLabel labelRes = new JLabel("Resolution");
		labelRes.setBounds(12, 190, 156, 15);
		// Create the combo box
		resolList = new JComboBox(resolStrings);
		resolList.setSelectedIndex(0);
		resolList.addActionListener(this);
		resolList.setActionCommand("Resolution");
		resolList.setBounds(200, 190, 150, 24);

		JButton sendButton = new JButton("Capture Image");
		sendButton.setBounds(95, 250, 150, 25);
		
		sendButton.addActionListener(this);
		sendButton.setActionCommand("Send");

		JButton streamButton = new JButton("Stream Off");
		streamButton.setBounds(295, 250, 207, 25);
		streamButton.addActionListener(this);
		streamButton.setActionCommand("Stream Off");

		panel.add(ipAddress); // Add IP address textfield to the panel
		panel.add(portnumber); // Add port textfield to the panel
		panel.add(fps); // Add fps textfield to the panel
		panel.add(labelipAddress); // Add IP address label to the panel
		panel.add(labelPort);// Add port label to the panel
		panel.add(labelFrequency);// Add frequency label to the panel
		panel.add(labelRes);// Add resolution label to the panel
		panel.add(resolList);// Add Resolution combo to the panel
		panel.add(sendButton);// Add Send Button to the panel
		panel.add(streamButton);// Add Send Button to the panel
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		
		if ("Send".equals(e.getActionCommand())) {
			try {
				if (selectedResolutionIndex == 0) {
					//resWidth = 176;
					//resHeight = 144;
					 JOptionPane.showMessageDialog(frame, "Select resolution", "Resolution warning",
					 JOptionPane.WARNING_MESSAGE);
				} else if (selectedResolutionIndex == 1) {
					resWidth = 1280;
					resHeight = 960;
				} else if (selectedResolutionIndex == 2) {
					resWidth = 1280;
					resHeight = 720;
				} else if (selectedResolutionIndex == 3) {
					resWidth = 1024;
					resHeight = 768;
				} else if (selectedResolutionIndex == 4) {
					resWidth = 1024;
					resHeight = 640;
				} else if (selectedResolutionIndex == 5) {
					resWidth = 640;
					resHeight = 480;
				} else if (selectedResolutionIndex == 6) {
					resWidth = 640;
					resHeight = 400;
				} else if (selectedResolutionIndex == 7) {
					resWidth = 320;
					resHeight = 240;
				} else if (selectedResolutionIndex == 8) {
					resWidth = 176;
					resHeight = 144;
				} else {
					JOptionPane.showMessageDialog(frame, "Choose resolution", "Resolution warning",
							JOptionPane.WARNING_MESSAGE);
				}

				if ((Integer.parseInt(fps.getText()) > 26)) {
					// custom title, warning icon
					JOptionPane.showMessageDialog(frame, "Enter fps less than 26", "FPS warning",
							JOptionPane.WARNING_MESSAGE);
				}

				// Create Socket
				createSocket(); //Socket created
				// Create data output stream
				dos = new DataOutputStream(socket.getOutputStream()); //Writing to the socket
				dis = new DataInputStream(socket.getInputStream());    //Reading from the socket
				// Sending client public key to the server
				sendRSAKeyToSocket();
				
				receiveRSAPubKey();   //Received RSA public key
				receiveSharedKey();    //Receive Shared key
				// Send DATA TO SERVER SOCKET
				sendFPStoSocket();

				streamThread = new Thread(new ClientUI());
				streamThread.start();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				
			}
		} else if ("Resolution".equals(e.getActionCommand())) {
			selectedResolutionIndex = resolList.getSelectedIndex();
			System.out.println("selectedResolutionIndex=" + selectedResolutionIndex);
		}
		if ("Stream Off".equals(e.getActionCommand())) {
			if (streamThread != null) {
				streamThread.stop();
			}

		} else {
			System.out.println("Unknown action");
		}
	}
      //Sending the RSA key to the socket
	private void sendRSAKeyToSocket() throws IOException {
		client = new RSA();
		client.generateKeyPair();
		dos.writeInt(client.n);
		dos.writeInt(client.e);
		dos.flush();
		System.out.println("Sending Client Modulus=" + client.n);
		System.out.println("Sending Client Public key Exponent=" + client.e);
	}

	public void sendFPStoSocket() throws IOException {
		int frequency = Integer.parseInt(fps.getText());
				
		//// ENCRYPT HERE
		
		int encryptedFrequency = XOR.encryptdecryptInt(frequency);
		
		
		System.out.println("secret key" + XOR.secretkey);
		
		int encryptedResWidth = XOR.encryptdecryptInt(resWidth);
		int encryptedResHeight = XOR.encryptdecryptInt(resHeight);
		
		dos.writeInt(encryptedFrequency);
		dos.writeInt(encryptedResWidth);
		dos.writeInt(encryptedResHeight);

		System.out.println("Sending frequency=" + frequency + " resWidth=" + resWidth + " resHeight=" + resHeight);
		System.out.println("Sending Encrypted frequency=" + encryptedFrequency + " resWidth=" + encryptedResWidth + " resHeight=" + encryptedResHeight);
		dos.flush();

		System.out.println("Exit : sendDatatoSocket");
	}

	private void receiveRSAPubKey() throws IOException {
		System.out.println("Entering receive RSA Public Key");
		server = new RSA();

		// Read first 4 bytes, int representing the modulus RSA key
		try {
			server.n = dis.readInt();
			System.out.println("Server Modulus= " + server.n);

			server.e = dis.readInt();
			System.out.println("Server Pub Exponent= " + server.e);

		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			isFinished = true;
		}
		System.out.println("Exiting from receive RSA Public Key");
	}

	private void receiveSharedKey() throws IOException {
		System.out.println("Entering : receiving Shared Key");
		XOR.setSecret(dis.readInt(), client);
		System.out.println("Exiting : received Shared Key");
	}

	public byte[] receiveEncByteArray() throws IOException {
		int arrayLen = 0;
		byte[] byteArray = null;
		System.out.println("Enter receiveByteArray");
		// Read first 4 bytes, int representing the length of the following
		// image
		try {
			
			arrayLen = dis.readInt();
			
			System.out.println("ARRAY SIZE ENC: " + arrayLen);
			System.out.println("SECRET KEY TO BE USED FOR SIZE: " + XOR.secretkey);

			arrayLen = XOR.encryptdecryptInt(arrayLen);	// Decrypting the size
			
			
			System.out.println("Array size: " + arrayLen);

			// Send confirmation


			System.out.println("imagelength= " + arrayLen);

			if (arrayLen == -55) {
				isFinished = true;
			}
		} catch (IOException e) {
			e.printStackTrace();
			isFinished = true;
		}

		if (arrayLen > 0) {
			
			// Read the image itself
			byteArray = new byte[arrayLen];
			int bytesRead = 0, totalBytesRead = 0;
			while (totalBytesRead < arrayLen) {
				bytesRead = dis.read(byteArray, totalBytesRead, arrayLen - totalBytesRead);
				totalBytesRead += bytesRead;
				System.out.println("bytesRead  = " + bytesRead);
			}
			System.out.println("totalBytesRead  = " + totalBytesRead);
			// imageArray = byteArray;
			
			
			for (int i = 0; i < byteArray.length; i++) {
				byteArray[i] =  XOR.decryptByte(byteArray[i]);
			}

			

		} // end of if
		else {
			
			isFinished = true;
		}
		

		  
		  
		return byteArray;
	}// end of function
	
	public static void main(String[] args) throws IOException, InterruptedException {
		System.out.println("Enter : main");
		// Create main panel for input values
		ClientUI mainWindow = new ClientUI();
		mainWindow.createMainWindow();
		mainWindow.setVisible(true);
		System.out.println("Exit : main");
	}

}
