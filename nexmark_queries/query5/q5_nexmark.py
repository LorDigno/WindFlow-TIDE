from windflow_tide import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../../include"),
    par= 3, 
    policy=TimePolicy.EVENT_TIME,
    time_baseline=("2026-09-01T00:00:00.000Z", TimeFormats.ISO8601)
)

#---- bid
bid_schema = (SchemaBuilder()
    .add_column("auction_id", DataTypes.BIGINT)             # Riferimento ad Auction
    .add_column("bidder", DataTypes.BIGINT)                 # ID utente (Person) che fa l'offerta
    .add_column("price", DataTypes.BIGINT)                  # Valore offerta (in centesimi, coerente con initial_bid/reserve)
    .add_column("channel", DataTypes.STRING)                # Canale di provenienza offerta
    .add_column("url", DataTypes.STRING)
    .add_column("bid_dateTime", TimeFormats.ISO8601)        # Timestamp dell'offerta
    .add_column("extra", DataTypes.STRING)                   # Campo di padding standard NEXMark
    .build()
)

bid_config = InputFileConfiguration(
    path = Path("../../nexmark_datasets/10m_bid.csv"),
    format = FileFormat.CSV,
    schema = bid_schema,
    has_header = True,
    time_col = "bid_dateTime",
    order = True,                                           # da vedere
    split_size= SplitSize.megabytes(32)
)

bid = env.table_from_file(bid_config, "bid_source")

#---    QUERY 5 
#--- Which auctions have seen the most bids in the last period?

#finestra temporale utilizzata
window = Window.createTBWindow(
    Duration.hours(12)
)

#quante puntate per ogni asta
bids_per_auction = (bid
    .name_query("bids_counter_per_auction")
    .group_by("auction_id", window= window)
    .select("auction_id", count().alias("bids_counter"))
)

#considero solo la slide corrente
slide_bucket = Window.createTBWindow(
    Duration.hours(12)
)

#numero di puntate per le aste più richieste
most_requested = (bids_per_auction
    .name_query("max_bid_count")
    .group_by(window= slide_bucket)
    .select(max("bids_counter").alias("maxxxxxx"))
)

#intervallo utilizzato per la join, necessario per il disallineamento dei ts
join_interval = Interval(
    Duration.hours(-1),
    Duration.hours(13)
)

#aste più richieste
hot_auctions = (bids_per_auction
    .rename_columns({"bids_counter": "maxxxxxx"})
    .name_query("auctions_with_max_bids")
    .join(most_requested, ["maxxxxxx"], attachment= join_interval)
    .select("auction_id", "maxxxxxx")
)

env.execute(hot_auctions, output_dir="./hot_ones")
