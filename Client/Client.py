import zmq
import tkinter as tk
from tkinter import messagebox

SERVER_PUSH_ADDR = "tcp://benternet.pxl-ea-ict.be:24041"
SERVER_SUB_ADDR = "tcp://benternet.pxl-ea-ict.be:24042"

class CoinLandClient:
    def __init__(self, root):
        self.root = root
        self.root.title("CoinLand Client - BasketbalGame & SlotMachine")
        self.name = ""
        self.context = zmq.Context()

        # Sockets
        self.push_socket = self.context.socket(zmq.PUSH)
        self.push_socket.connect(SERVER_PUSH_ADDR)

        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(SERVER_SUB_ADDR)
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, "Emiel>Leaderboard!>")  # Leaderboard subscriben

        self.leaderboard_data = []  # Om leaderboard info lokaal bij te houden

        self.build_gui()

        # Start response polling
        self.root.after(100, self.poll_response)

    def build_gui(self):
        tk.Label(self.root, text="Spelersnaam:").pack()
        self.name_entry = tk.Entry(self.root)
        self.name_entry.pack()

        # BasketbalGame
        tk.Label(self.root, text="Gooi een getal (1-10) voor de BasketbalGame:").pack()
        self.guess_entry = tk.Entry(self.root)
        self.guess_entry.pack()
        tk.Button(self.root, text="Speel BasketbalGame", command=self.play_basketbal_game).pack(pady=5)

        # SlotMachine
        tk.Button(self.root, text="Speel SlotMachine", command=self.play_slot_machine).pack(pady=5)

        # Leaderboard knop
        tk.Button(self.root, text="Toon Leaderboard", command=self.show_leaderboard_popup).pack(pady=10)

        self.output_box = tk.Text(self.root, height=12, state="disabled")
        self.output_box.pack(pady=10)

    def play_basketbal_game(self):
        self.name = self.name_entry.get().strip()
        guess = self.guess_entry.get().strip()
        if not self.name or not guess.isdigit():
            messagebox.showerror("Fout", "Vul een geldige naam en getal in.")
            return
        if not (1 <= int(guess) <= 10):
            messagebox.showerror("Fout", "Getal moet tussen 1 en 10 liggen.")
            return

        topic = f"Emiel>BasketbalGame!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Emiel>BasketbalGame?>{self.name}>{guess}>"
        self.push_socket.send_string(message)
        self.output(f"[BasketbalGame] Gok verzonden...")

    def play_slot_machine(self):
        self.name = self.name_entry.get().strip()
        if not self.name:
            messagebox.showerror("Fout", "Voer een geldige naam in.")
            return

        topic = f"Emiel>BasketbalMachine!>{self.name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, topic)

        message = f"Emiel>BasketbalMachine?>{self.name}>"
        self.push_socket.send_string(message)
        self.output(f"[SlotMachine] Draai gestart...")

    def poll_response(self):
        try:
            while True:
                msg = self.sub_socket.recv_string(flags=zmq.NOBLOCK)
                if msg.startswith("Emiel>Leaderboard!>"):
                    self.update_leaderboard(msg)
                else:
                    self.output(msg)
        except zmq.Again:
            pass
        self.root.after(100, self.poll_response)

    def output(self, tekst):
        self.output_box.config(state="normal")
        self.output_box.insert(tk.END, tekst + "\n")
        self.output_box.see(tk.END)
        self.output_box.config(state="disabled")

    def update_leaderboard(self, msg):
        # Parse leaderboard string: "Emiel>Leaderboard!>speler1:score|speler2:score|..."
        leaderboard_data = msg.split(">", 2)[2]  # alles na 'Emiel>Leaderboard!>'
        spelers = leaderboard_data.strip("|").split("|")

        # Lijst opslaan voor gebruik in popup
        self.leaderboard_data = []
        for speler_info in spelers:
            if ":" in speler_info:
                naam, score_str = speler_info.split(":")
                try:
                    score = int(score_str)
                    self.leaderboard_data.append((naam, score))
                except ValueError:
                    pass

        # Sorteer descending op score (voor zekerheid)
        self.leaderboard_data.sort(key=lambda x: x[1], reverse=True)

        # Update popup als die open is
        if hasattr(self, "leaderboard_popup") and self.leaderboard_popup.winfo_exists():
            self.refresh_leaderboard_popup()

    def show_leaderboard_popup(self):
        if hasattr(self, "leaderboard_popup") and self.leaderboard_popup.winfo_exists():
            # Al open? Breng naar voren
            self.leaderboard_popup.lift()
            return

        self.leaderboard_popup = tk.Toplevel(self.root)
        self.leaderboard_popup.title("Leaderboard")

        self.leaderboard_text = tk.Text(self.leaderboard_popup, height=15, width=40, state="disabled", font=("Consolas", 12))
        self.leaderboard_text.pack(padx=10, pady=10)

        self.refresh_leaderboard_popup()

    def refresh_leaderboard_popup(self):
        self.leaderboard_text.config(state="normal")
        self.leaderboard_text.delete("1.0", tk.END)

        # Toon alle spelers met positie, en markeer jezelf vetgedrukt
        your_name = self.name_entry.get().strip()
        if not your_name:
            your_name = None

        for i, (naam, score) in enumerate(self.leaderboard_data, start=1):
            line = f"{i}. {naam}: {score} punten\n"
            if naam == your_name:
                self.leaderboard_text.insert(tk.END, line, "highlight")
            else:
                self.leaderboard_text.insert(tk.END, line)

        self.leaderboard_text.tag_config("highlight", font=("Consolas", 12, "bold"))
        self.leaderboard_text.config(state="disabled")


if __name__ == "__main__":
    root = tk.Tk()
    app = CoinLandClient(root)
    root.mainloop()
