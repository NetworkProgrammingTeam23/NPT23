import java.io.*;
import java.net.*;
import java.util.Scanner;

public class Gitclient {
    private static boolean clientRunning = true;
    private static long sendTime = 0; // 메시지 송신 시간을 저장할 변수
    public static void main(String[] args) throws IOException {
        String serverName = "127.0.0.1";
        int port = 9210;
        try {
            System.out.println("Connecting to " + serverName + " on port " + port);
            Socket client = new Socket(serverName, port);

            System.out.println("Just connected to " + client.getRemoteSocketAddress());
            OutputStream outToServer = client.getOutputStream();
            DataOutputStream out = new DataOutputStream(outToServer);

            System.out.print("Enter your nickname: ");
            Scanner scanner = new Scanner(System.in);
            String nickname = scanner.nextLine();
            out.write(nickname.getBytes("EUC-KR"));

            Thread sendThread = new Thread(() -> {
            	try {
                    while (clientRunning) {
                        String sendText = scanner.nextLine();
                        sendTime = System.nanoTime(); // 메시지 송신 시간 기록
                        out.write(sendText.getBytes("EUC-KR"));
                        if (sendText.equals("quit")) {
                            clientRunning = false;
                            client.close();
                            scanner.close();
                            System.exit(0);
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
            sendThread.start();

            Thread receiveThread = new Thread(() -> {
                try {
                    InputStream inFromServer = client.getInputStream();
                    while (clientRunning) {
                        byte[] buffer = new byte[1024];
                        int bytesReceived = inFromServer.read(buffer);
                        if (bytesReceived == -1) {
                            break;
                        }
                        long receivedTime = System.nanoTime(); // 메시지 수신 시간 기록
                        double elapsedTime = (receivedTime - sendTime) / 1e9; // 초 단위로 변환
                        System.out.printf("메시지 수신 시간:%.7f초\n ",elapsedTime);
                        String receivedText = new String(buffer, 0, bytesReceived, "EUC-KR");
                        if (receivedText.equals("quit")) {
                            clientRunning = false;
                            client.close();
                            System.exit(0);
                        }
                        System.out.println(receivedText);
                    }
                } catch (SocketException e) {
                    System.out.println("Connection was reset. Exiting...");
                    System.exit(0);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            });
            receiveThread.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}