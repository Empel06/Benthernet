import zmq
import tkinter as tk
from tkinter import messagebox

SERVER_PUSH_ADDR = "tcp://benternet.pxl-ea-ict.be:24041"
SERVER_SUB_ADDR = "tcp://benternet.pxl-ea-ict.be:24042"

class CoinLandClient:
    def __init__(self, root):
        self.root = root
        self.root.title("Emiel's BasketbalGame & BasketbalMachine")
        self.name = ""
        self.context = zmq.Context()
        self.hoogste_score_ooit = ("", 0)
        
        self.root.configure(bg="#001f4d")

        # PUSH socket voor verzenden
        self.push_socket = self.context.socket(zmq.PUSH)
        self.push_socket.connect(SERVER_PUSH_ADDR)

        # SUB socket voor ontvangen
        self.sub_socket = self.context.socket(zmq.SUB)
        self.sub_socket.connect(SERVER_SUB_ADDR)
        # Subscribe naar leaderboard en op naam basis
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, "Emiel>Leaderboard!>")

        self.leaderboard_data = []
        self.subscribed_names = set()

        self.build_gui()
        
        self.double_button.config(state="disabled")  # Disabled by default

        self.root.after(100, self.poll_response)

    def build_gui(self):
        tk.Label(self.root, text="Spelersnaam:", font=("Segoe UI", 12), fg="white", bg="#001f4d").pack(pady=(10,0))
        self.name_entry = tk.Entry(self.root, font=("Segoe UI", 12), bg="#d9d9d9")
        self.name_entry.pack(pady=(0,10), ipadx=5, ipady=5)

        tk.Label(self.root, text="Gooi een getal (1-10) voor de BasketbalGame:", font=("Segoe UI", 11), fg="white", bg="#001f4d").pack()
        self.guess_entry = tk.Entry(self.root, font=("Segoe UI", 12), bg="#d9d9d9")
        self.guess_entry.pack(pady=(0,10), ipadx=5, ipady=5)

        tk.Label(self.root, text="Gok het winnende team voor BasketbalMachine:", font=("Segoe UI", 11), fg="white", bg="#001f4d").pack()
        self.team_guess_entry = tk.Entry(self.root, font=("Segoe UI", 12), bg="#d9d9d9")
        self.team_guess_entry.pack(pady=(0,10), ipadx=5, ipady=5)
        tk.Label(self.root, text="Teams: Lakers, Celtics", font=("Segoe UI", 9), fg="white", bg="#001f4d").pack(pady=(0,10))

        button_style = {
            "font": ("Segoe UI", 12, "bold"),
            "bg": "#d9d9d9",
            "fg": "black",
            "activebackground": "#c0c0c0",
            "activeforeground": "black",
            "relief": "raised",
            "bd": 2,
            "width": 25
        }

        self.double_button = tk.Button(self.root, text="Speel DoubleOrNothing", command=self.play_double_or_nothing, **button_style)
        self.double_button.pack(pady=5)

        tk.Button(self.root, text="Speel BasketbalGame", command=self.play_basketbal_game, **button_style).pack(pady=5)
        tk.Button(self.root, text="Speel BasketbalMachine", command=self.play_basketbal_machine, **button_style).pack(pady=5)
        tk.Button(self.root, text="Toon Leaderboard", command=self.show_leaderboard_popup, **button_style).pack(pady=15)

        self.output_box = tk.Text(self.root, height=12, font=("Consolas", 11), bg="#d9d9d9", state="disabled")
        self.output_box.pack(padx=10, pady=10, fill="both", expand=True)

    def subscribe_to_name_topics(self, name):
        # Subscribe alleen als nog niet gedaan
        if name in self.subscribed_names:
            return
        game_topic = f"Emiel>BasketbalGame!>{name}>"
        machine_topic = f"Emiel>BasketbalMachine!>{name}>"
        double_topic = f"Emiel>DoubleOrNothing!>{name}>"
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, game_topic)
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, machine_topic)
        self.sub_socket.setsockopt_string(zmq.SUBSCRIBE, double_topic)
        self.subscribed_names.add(name)

    def play_basketbal_game(self):
        naam = self.name_entry.get().strip()
        guess = self.guess_entry.get().strip()
        if not naam or not guess.isdigit():
            messagebox.showerror("Fout", "Vul een geldige naam en getal in.")
            return
        if not (1 <= int(guess) <= 10):
            messagebox.showerror("Fout", "Getal moet tussen 1 en 10 liggen.")
            return

        self.name = naam
        self.subscribe_to_name_topics(self.name)

        message = f"Emiel>BasketbalGame?>{self.name}>{guess}>"
        self.push_socket.send_string(message)
        self.output(f"[BasketbalGame] Gok verzonden voor {self.name} met getal {guess}...")

    def play_basketbal_machine(self):
        naam = self.name_entry.get().strip()
        gok_team = self.team_guess_entry.get().strip()
        if not naam:
            messagebox.showerror("Fout", "Voer een geldige naam in.")
            return
        if not gok_team:
            messagebox.showerror("Fout", "Voer een geldig gokteam in.")
            return

        valid_teams = {"Lakers", "Celtics"}
        if gok_team not in valid_teams:
            messagebox.showerror("Fout", f"Ongeldig team. Kies uit: {', '.join(valid_teams)}")
            return

        self.name = naam
        self.subscribe_to_name_topics(self.name)

        message = f"Emiel>BasketbalMachine?>{self.name}>{gok_team}>"
        self.push_socket.send_string(message)
        self.output(f"[BasketbalMachine] Wedstrijd gok verzonden voor {self.name} op team {gok_team}...")

    def poll_response(self):
        try:
            while True:
                msg = self.sub_socket.recv_string(flags=zmq.NOBLOCK)
                if msg.startswith("Emiel>Leaderboard!>"):
                    self.update_leaderboard(msg)
                    self.update_double_button_state()
                elif msg.startswith("Emiel>DoubleOrNothing!>"):
                    self.handle_double_or_nothing_response(msg)
                else:
                    # Check of bericht voor deze speler is
                    if self.name and (f">{self.name}>" in msg):
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
        # msg = "Emiel>Leaderboard!>naam1:score1|naam2:score2|..."
        leaderboard_data = msg.split(">", 2)[2]
        spelers = leaderboard_data.strip("|").split("|")

        self.leaderboard_data = []
        for speler_info in spelers:
            if ":" in speler_info:
                naam, score_str = speler_info.split(":")
                try:
                    score = int(score_str)
                    self.leaderboard_data.append((naam, score))

                    if score > self.hoogste_score_ooit[1]:
                        self.hoogste_score_ooit = (naam, score)

                except ValueError:
                    pass

        self.leaderboard_data.sort(key=lambda x: x[1], reverse=True)

        if hasattr(self, "leaderboard_popup") and self.leaderboard_popup.winfo_exists():
            self.refresh_leaderboard_popup()

    def show_leaderboard_popup(self):
        if hasattr(self, "leaderboard_popup") and self.leaderboard_popup.winfo_exists():
            self.leaderboard_popup.lift()
            return

        self.leaderboard_popup = tk.Toplevel(self.root)
        self.leaderboard_popup.title("Leaderboard")

        self.leaderboard_text = tk.Text(self.leaderboard_popup, height=15, width=40, state="disabled",
                                        font=("Consolas", 12))
        self.leaderboard_text.pack(padx=10, pady=10)

        self.refresh_leaderboard_popup()

    def refresh_leaderboard_popup(self):
        self.leaderboard_text.config(state="normal")
        self.leaderboard_text.delete("1.0", tk.END)

        hoogste_speler, hoogste_score = self.hoogste_score_ooit
        hoogste_tekst = f"Hoogste score ooit: {hoogste_speler} met {hoogste_score} punten\n\n"
        self.leaderboard_text.insert(tk.END, hoogste_tekst)

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

    def play_double_or_nothing(self):
        naam = self.name_entry.get().strip()
        if not naam:
            messagebox.showerror("Fout", "Voer een geldige naam in.")
            return

        # Score ophalen van de speler
        speler_score = next((score for n, score in self.leaderboard_data if n == naam), None)
        if speler_score < 50:
            messagebox.showerror("Fout", "Je moet minstens 50 punten hebben om DoubleOrNothing te spelen.")
            return


        self.name = naam
        self.subscribe_to_name_topics(self.name)
        message = f"Emiel>DoubleOrNothing?>{self.name}>"
        self.push_socket.send_string(message)
        self.output(f"[DoubleOrNothing] Speler {self.name} speelt DoubleOrNothing...")

    def handle_double_or_nothing_response(self, msg):
        # msg formaat: Emiel>DoubleOrNothing!>naam>resultaat>
        parts = msg.split(">")
        if len(parts) < 4:
            return
        naam = parts[2]
        resultaat = parts[3]
        if resultaat == "gewonnen":
            tekst = f"Gefeliciteerd {naam}, je hebt DoubleOrNothing gewonnen! Je punten zijn verdubbeld."
        elif resultaat == "verloren":
            tekst = f"Helaas {naam}, je hebt DoubleOrNothing verloren en je punten zijn teruggezet naar 0."
        else:
           tekst = f"DoubleOrNothing bericht ontvangen: {msg}"

        self.output(tekst)

        self.update_double_button_state()

    def update_double_button_state(self):
        naam = self.name_entry.get().strip()
        speler_score = next((score for n, score in self.leaderboard_data if n == naam), None)
        if speler_score and speler_score >= 50:
            self.double_button.config(state="normal")
        else:
            self.double_button.config(state="disabled")



if __name__ == "__main__":
    root = tk.Tk()
    app = CoinLandClient(root)
    root.mainloop()
