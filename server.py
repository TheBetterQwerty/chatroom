import socket
import threading
import os 

HOST =  '0.0.0.0'
PORT = 6969
LISTENER_LIMIT = 5
active_clients = []

def listen_for_messages(client, username):
    try:
        while True:
            message = client.recv(2048).decode('utf-8')
            if not message:
                break
            final_msg = username + '~' + message
            send_messages_to_all(final_msg)
    except Exception as e:
        print(f"Error in listening for messages from {username}: {e}")
    finally:
        client.close()
        remove_client(username)

def send_message_to_client(client, message):
    try:
        client.sendall(message.encode())
    except Exception as e:
        print(f"Error sending message to client: {e}")

def send_messages_to_all(message):
    for user in active_clients[:]:  # Use a copy of the list to avoid issues during iteration
        send_message_to_client(user[1], message)

def remove_client(username):
    for user in active_clients[:]:  # Use a copy of the list to avoid issues during iteration
        if user[0] == username:
            active_clients.remove(user)
            prompt_message = "SERVER~" + f"{username} has left the chat"
            send_messages_to_all(prompt_message)

def client_handler(client):
    try:
        username = client.recv(2048).decode('utf-8')
        if username:
            active_clients.append((username, client))
            prompt_message = "SERVER~" + f"{username} added to the chat"
            send_messages_to_all(prompt_message)
            threading.Thread(target=listen_for_messages, args=(client, username)).start()
    except Exception as e:
        print(f"Error handling client: {e}")
        client.close()

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        server.bind((HOST, 6969))
        print(f"Running the server on {HOST} {PORT}")
    except Exception as e:
        print(f"Unable to bind to host {HOST} and port {PORT}: {e}")
        return

    server.listen(LISTENER_LIMIT)

    try:
        while True:
            client, address = server.accept()
            print(f"Successfully connected to client {address[0]} {address[1]}")
            threading.Thread(target=client_handler, args=(client,)).start()
    except KeyboardInterrupt:
        print("Server shutting down.")
    except Exception as e:
        print(f"Error in main loop: {e}")
    finally:
        server.close()

if __name__ == '__main__':
    main()
