from windflow_table_api import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../../include"),
    par= 5, 
    policy=TimePolicy.INGRESS_TIME,
    #time_baseline=("2026-09-01T00:00:00.000Z", TimeFormats.ISO8601)
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
    path = Path("../../nexmark_datasets/30m_bid.csv"),
    format = FileFormat.CSV,
    schema = bid_schema,
    has_header = True,
    #time_col = "bid_dateTime",
    order = True,                                           # da vedere
    split_size= SplitSize.megabytes(32)
)

bid = env.table_from_file(bid_config, "bid_source")


#---    QUERY 12
#--- How many bids does a user make within a fixed processing time limit? 
#--- Preambolo diverso dato che questa usa INGRESS_TIME

process_window = Window.createTBWindow(
    Duration.milliseconds(1000)
)

q12 = (bid
    .name_query("ingress_time_windows")
    .group_by("bidder", window= process_window)
    .select("bidder", count().alias("processed"))
)

env.execute(q12, output_dir= "./ingress")

