from windflow_table_api import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../../include"),
    par= 2, 
    policy=TimePolicy.EVENT_TIME,
    time_baseline=("2026-09-01T00:00:00.000Z", TimeFormats.ISO8601)
)

#---- auction
auction_schema = (SchemaBuilder()
    .add_column("auction_id", DataTypes.BIGINT)
    .add_column("item_name", DataTypes.STRING)
    .add_column("description", DataTypes.STRING)
    .add_column("initial_bid", DataTypes.BIGINT)
    .add_column("reserve", DataTypes.BIGINT)
    .add_column("auction_dateTime", TimeFormats.ISO8601)    #timestamp
    .add_column("expires", TimeFormats.ISO8601)
    .add_column("seller", DataTypes.BIGINT)                 #id di un Person
    .add_column("category", DataTypes.BIGINT)
    .build()
)

auction_config = InputFileConfiguration(
    path = Path("../../nexmark_datasets/70m_auction.csv"),
    format= FileFormat.CSV,
    schema= auction_schema,
    has_header= True,
    time_col= "auction_dateTime",
    order= True,                        
    split_size= SplitSize.megabytes(64)
)

auction = env.table_from_file(auction_config, "auction_source")

base = auction.name_query("discard_a").select("auction_id", "item_name", "description", "initial_bid", "reserve", 
                      "auction_dateTime", "expires", "seller", "category")

env.execute(base, output_dir= "./auction")